import serial
import serial.tools.list_ports
import math
import pygame
import sys

DEFAULT_PORTS = ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyUSB0', '/dev/ttyUSB1', 'COM3', 'COM4']
BAUD_RATE = 9600

def connect_serial():
    ports = [p.device for p in serial.tools.list_ports.comports()]
    all_candidate_ports = ports + [p for p in DEFAULT_PORTS if p not in ports]
    
    for port in all_candidate_ports:
        try:
            s = serial.Serial(port, BAUD_RATE, timeout=0.05)
            s.reset_input_buffer()
            print(f"Connecté avec succès au port série : {port}")
            return s
        except Exception:
            continue
    return None

pygame.init()
WIDTH, HEIGHT = 800, 500
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Radar sonar 2D - Prototypage IA & Robotique")

GREEN = (0, 255, 0)
DARK_GREEN = (0, 50, 0)
RED = (255, 0, 0)
BLACK = (0, 0, 0)

CENTER = (WIDTH // 2, HEIGHT - 50)
MAX_DISTANCE = 100
SCALE = (HEIGHT - 100) / MAX_DISTANCE

ser = connect_serial()
if ser is None:
    print("Erreur : Impossible d'ouvrir un port série valide (ex: /dev/ttyACM0). Vérifiez les branchements.")
    sys.exit(1)

history = {}
current_angle = 0
clock = pygame.time.Clock()
running = True

while running:
    # 1. Toujours traiter les événements système en PREMIER
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # 2. Lecture de TOUTES les lignes en attente dans le buffer série
    while ser.in_waiting > 0:
        try:
            line_bytes = ser.readline()
            raw_line = line_bytes.decode('utf-8', errors='ignore').strip()
            if not raw_line:
                continue
            
            # Support du format 'angle,distance' (avec ou sans point final)
            cleaned = raw_line.rstrip('.')
            if ',' in cleaned:
                parts = cleaned.split(',')
                if len(parts) >= 2:
                    angle = int(parts[0].strip())
                    dist = int(parts[1].strip())
                    history[angle] = dist
                    current_angle = angle
        except (ValueError, IndexError):
            pass

    # 3. Rendu de l'écran
    screen.fill(BLACK)

    # Arcs de cercle
    for r in range(25, MAX_DISTANCE + 1, 25):
        radius = int(r * SCALE)
        pygame.draw.arc(screen, DARK_GREEN, (CENTER[0] - radius, CENTER[1] - radius, radius * 2, radius * 2), 0, math.pi, 2)

    # Grille d'angles
    for a in [0, 45, 90, 135, 180]:
        rad = math.radians(a)
        x = CENTER[0] - int(math.cos(rad) * (MAX_DISTANCE * SCALE))
        y = CENTER[1] - int(math.sin(rad) * (MAX_DISTANCE * SCALE))
        pygame.draw.line(screen, DARK_GREEN, CENTER, (x, y), 1)

    # Balayage dynamique
    sweep_rad = math.radians(current_angle)
    x_sweep = CENTER[0] - int(math.cos(sweep_rad) * (MAX_DISTANCE * SCALE))
    y_sweep = CENTER[1] - int(math.sin(sweep_rad) * (MAX_DISTANCE * SCALE))
    pygame.draw.line(screen, GREEN, CENTER, (x_sweep, y_sweep), 2)

    # Echo des obstacles
    for angle, dist in list(history.items()):
        rad = math.radians(angle)
        if 0 < dist < MAX_DISTANCE:
            x_obj = CENTER[0] - int(math.cos(rad) * (dist * SCALE))
            y_obj = CENTER[1] - int(math.sin(rad) * (dist * SCALE))
            pygame.draw.circle(screen, RED, (x_obj, y_obj), 4)

    pygame.display.flip()
    clock.tick(60) # Cadencement fixe à 60 FPS

ser.close()
pygame.quit()