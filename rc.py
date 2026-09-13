import socket
import json
import time
import pygame

# --- CONFIGURATION ---
CAR_ADDRESS = ("10.147.213.72", 5005)  
TIMEOUT_SEC = 0.1  
MAX_RETRIES = 5
UPDATE_RATE_HZ = 20  # Sends 20 packets per second

# --- UDP SETUP ---
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 5006)) 

def send_reliable_command(seq_num, command):
    packet = json.dumps({"type": "cmd", "seq": seq_num, "val": command}).encode('utf-8')
    sock.settimeout(TIMEOUT_SEC)
    for attempt in range(MAX_RETRIES):
        sock.sendto(packet, CAR_ADDRESS)
        try:
            data, _ = sock.recvfrom(1024)
            if json.loads(data.decode('utf-8')).get("ack_seq") == seq_num:
                print(f"[CMD] Success: '{command}' acknowledged.")
                return True
        except socket.timeout:
            pass
    print(f"[CMD] Failed to deliver '{command}'")
    return False

def send_continuous_control(throttle, steering):
    packet = json.dumps({"type": "stream", "thr": throttle, "str": steering}).encode('utf-8')
    sock.sendto(packet, CAR_ADDRESS)

# --- PYGAME / KEYBOARD SETUP ---
pygame.init()

# A small window is required for Pygame to capture keyboard events on Windows
screen = pygame.display.set_mode((300, 100))
pygame.display.set_caption("RC Car Teleop (WASD)")

# Draw a simple instruction label so the window isn't blank
font = pygame.font.SysFont(None, 22)
label = font.render("WASD=Drive  SPACE=Lights  ESC=Quit", True, (220, 220, 220))
screen.fill((30, 30, 30))
screen.blit(label, (10, 40))
pygame.display.flip()

# --- MAIN TELEOP LOOP ---
print("Teleop started (WASD + SPACE). Keep the Pygame window focused!")
print("  W/S = Throttle  |  A/D = Steering  |  SPACE = Headlights  |  ESC = Quit")
seq_counter = 1
space_last_state = False  # For edge-detection on spacebar

try:
    while True:
        # Process Pygame events (required for key state to update)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                raise KeyboardInterrupt

        # 1. Read currently-held keys
        keys = pygame.key.get_pressed()

        # ESC to quit cleanly
        if keys[pygame.K_ESCAPE]:
            raise KeyboardInterrupt

        # Throttle: W = forward 100, S = reverse -100, neither = 0
        if keys[pygame.K_w]:
            throttle = 100
        elif keys[pygame.K_s]:
            throttle = -100
        else:
            throttle = 0

        # Steering: A = left -100, D = right 100, neither = 0
        if keys[pygame.K_a]:
            steering = -100
        elif keys[pygame.K_d]:
            steering = 100
        else:
            steering = 0

        # 2. Send Continuous Data
        send_continuous_control(throttle, steering)
        print(f"Streaming -> Throttle: {throttle}% | Steering: {steering}%", end='\r')

        # 3. Spacebar for Reliable Command (Edge Detection — fires once per press)
        space_current = keys[pygame.K_SPACE]

        if space_current and not space_last_state:
            print("\n")  # Break the carriage return line
            send_reliable_command(seq_counter, "HEADLIGHTS_TOGGLE")
            seq_counter += 1

        space_last_state = space_current

        # 4. Enforce Update Rate
        time.sleep(1.0 / UPDATE_RATE_HZ)

except KeyboardInterrupt:
    print("\nTeleop stopped. Sending zero-command to halt car.")
    send_continuous_control(0, 0)
    pygame.quit()
