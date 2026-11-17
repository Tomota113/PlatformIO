import csv
import time
import serial

# Configuration du port
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200
OUTPUT_FILE = "motor_dataset.csv"

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
time.sleep(2)

print("=== ENREGISTREUR DE DONNEES TINYML ===")
label = input("Entrez la classe à enregistrer (0 = NORMAL, 1 = ANOMALIE) : ")
nb_samples = int(input("Nombre d'échantillons à capturer (ex: 200) : "))

data_collected = []
print("\nDébut de la capture... Laissez le système tourner.")

while len(data_collected) < nb_samples:
    try:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if "RMS:" in line and "Freq:" in line:
            # Parsing de la ligne : RMS: X | Peak: Y | Freq: Z | Temp: T
            parts = line.split("|")
            rms = float(parts[0].split(":")[1].replace("g", "").strip())
            peak = float(parts[1].split(":")[1].replace("g", "").strip())
            freq = float(parts[2].split(":")[1].replace("Hz", "").strip())
            temp = float(parts[3].split(":")[1].replace("°C", "").strip())

            sample = [rms, peak, freq, temp, int(label)]
            data_collected.append(sample)
            print(f"[{len(data_collected)}/{nb_samples}] Capturé : {sample[:-1]}")
    except Exception as e:
        continue

# Sauvegarde dans le CSV
with open(OUTPUT_FILE, mode="a", newline="") as f:
    writer = csv.writer(f)
    # Écrire l'en-tête si le fichier est vide
    if f.tell() == 0:
        writer.writerow(["rms", "peak", "freq", "temp", "label"])
    writer.writerows(data_collected)

print(f"\n[SUCCÈS] {nb_samples} échantillons enregistrés pour la classe {label} dans {OUTPUT_FILE}.")
ser.close()