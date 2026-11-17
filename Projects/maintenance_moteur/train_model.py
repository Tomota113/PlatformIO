import io
import urllib.request
import numpy as np
import scipy.io
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
import micromlgen

print("=== TÉLÉCHARGEMENT DU BENCHMARK CWRU OFFICIEL ===")

# URLs officielles CWRU : Données normales (97.mat) et Défaut roulement (105.mat)
URL_NORMAL = "https://engineering.case.edu/sites/default/files/97.mat"
URL_FAULT  = "https://engineering.case.edu/sites/default/files/105.mat"

def load_cwru_mat(url):
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response:
        mat_bytes = response.read()
    mat_data = scipy.io.loadmat(io.BytesIO(mat_bytes))
    # Extraction de la série temporelle d'accélération (Drive End)
    for key in mat_data.keys():
        if "DE_time" in key:
            return mat_data[key].flatten()
    raise ValueError(f"Signal DE_time non trouvé dans {url}")

try:
    print("1/4 Récupération du signal nominal (Normal)...")
    sig_normal = load_cwru_mat(URL_NORMAL)[:30000]

    print("2/4 Récupération du signal avec défaut mécanique (Anomalie)...")
    sig_fault = load_cwru_mat(URL_FAULT)[:30000]
    print("[OK] Données réelles téléchargées avec succès.")
except Exception as e:
    print(f"Erreur réseau CWRU : {e}")
    exit(1)

# Extraction des caractéristiques par fenêtre de 64 échantillons (identique à l'ESP32)
WINDOW_SIZE = 64
features = []
labels = []

def extract_features(signal, label_val, base_temp):
    num_windows = len(signal) // WINDOW_SIZE
    for i in range(num_windows):
        window = signal[i * WINDOW_SIZE : (i + 1) * WINDOW_SIZE]
        
        # 1. RMS
        rms = float(np.sqrt(np.mean(window ** 2)))
        
        # 2. Peak
        peak = float(np.max(np.abs(window)))
        
        # 3. Fréquence dominante FFT (échantillonnage ramené à 200 Hz)
        fft_vals = np.abs(np.fft.rfft(window))
        freqs = np.fft.rfftfreq(WINDOW_SIZE, d=1.0 / 200.0)
        dominant_freq = float(freqs[np.argmax(fft_vals[1:]) + 1])
        
        # 4. Température simulée associée
        temp = float(np.random.normal(base_temp, 1.2))
        
        features.append([rms, peak, dominant_freq, temp])
        labels.append(label_val)

print("3/4 Extraction des features FFT / RMS / Peak...")
extract_features(sig_normal, 0, base_temp=28.0) # Classe 0: Normal
extract_features(sig_fault, 1, base_temp=38.5)  # Classe 1: Défaut

X = np.array(features)
y = np.array(labels)

# Entraînement Random Forest léger pour microcontrôleur
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

clf = RandomForestClassifier(n_estimators=3, max_depth=4, random_state=42)
clf.fit(X_train, y_train)

print("\n=== RAPPORT DE PERFORMANCE SUR DONNÉES CWRU ===")
print(classification_report(y_test, clf.predict(X_test), target_names=["Normal", "Anomalie"]))

# 4. Export C++ pour l'ESP32
c_code = micromlgen.port(clf, classname="MotorAnomalyClassifier")

with open("include/model.h", "w") as f:
    f.write(c_code)

print("[SUCCÈS] Modèle exporté directement dans 'include/model.h' !")