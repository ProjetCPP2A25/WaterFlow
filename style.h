#pragma once

// Feuille de style unique de WaterFlow (fusion des styles Personnel + Livraisons).
// Les règles propres à un module sont limitées par objectName / nom de classe
// afin qu'elles ne modifient pas l'apparence de l'autre module.
inline const char *const kStyleSheet = R"(
/* ---------- Base ---------- */
QWidget { background:#F4F7FB; color:#172B4D; font-family:"Segoe UI"; font-size:9pt; }
QLabel { background:transparent; }
QLabel#labelTitre { color:#122344; font-size:21pt; font-weight:800; }
Livraisons QLabel#labelTitre { padding-bottom:5px; }

/* ---------- Barre latérale ---------- */
QFrame#sidebar { background:#101F3C; border:none; }
QLabel#labelLogo { color:white; font-size:17pt; font-weight:800; padding:8px 4px 18px 4px; }
QListWidget#menuLateral { background:transparent; color:#B8C5DB; border:none; font-size:10pt; outline:none; padding-top:8px; }
QListWidget#menuLateral::item { padding:12px 12px; border-radius:8px; margin:3px 0; }
QListWidget#menuLateral::item:hover { background:#1C3156; color:white; }
QListWidget#menuLateral::item:selected { background:#2878F0; color:white; font-weight:600; }

/* ---------- Page « bientôt disponible » ---------- */
QLabel#placeholderSub { color:#75839A; font-size:11pt; }

/* ---------- Cartes / alertes (Personnel) ---------- */
QFrame#frameAlerte { background:#FFF9EC; border:1px solid #F6E5BE; border-left:4px solid #E9A23B; border-radius:10px; }
QFrame#cardListe, QFrame#cardFiche, QFrame#cardPlanning, QFrame#cardGraphique { background:#FFFFFF; border:1px solid #E4EAF2; border-radius:14px; }
QLabel#labelTitreListe, QLabel#labelTitreFiche, QLabel#labelTitrePlanning, QLabel#labelTitreGraphique { color:#1B3157; font-weight:700; font-size:10pt; padding-bottom:3px; }

/* ---------- Boutons ---------- */
QPushButton { background:#2878F0; color:white; border:none; border-radius:8px; padding:9px 15px; font-weight:650; }
QPushButton:hover { background:#1765D1; }
QPushButton:pressed { background:#124FA7; }
QPushButton#btnAjouter { background:#EAF2FF; color:#2169D5; border:1px solid #D4E4FF; }
QPushButton#btnAjouter:hover { background:#DCEAFF; }
QPushButton#btnEnregistrer, QPushButton#btnModifier { background:#2878F0; }
QPushButton#btnSupprimer { background:#FFF0F0; color:#C03945; border:1px solid #F8D9DC; }
QPushButton#btnSupprimer:hover { background:#FFE1E3; }
QPushButton#btnExporter, QPushButton#btnPdf { background:#14294D; }
QPushButton#btnExporter:hover, QPushButton#btnPdf:hover { background:#233D68; }

/* ---------- Champs de saisie ---------- */
QLineEdit, QComboBox, QDateEdit, QDateTimeEdit { background:#FFFFFF; border:1px solid #DCE4EF; border-radius:8px; padding:9px 10px; selection-background-color:#2878F0; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDateTimeEdit:focus { border:1px solid #2878F0; }
QComboBox::drop-down { border:none; width:24px; }

/* ---------- Tableaux ---------- */
QTableWidget { background:white; border:none; alternate-background-color:#F7F9FC; selection-background-color:#E6F0FF; selection-color:#172B4D; gridline-color:#EEF2F7; outline:none; }
QTableWidget::item { padding-left:7px; border-bottom:1px solid #F0F3F8; }
QTableWidget::item:selected { color:#172B4D; }
QTableView#tableView { background:white; border:none; alternate-background-color:#F7F9FC; selection-background-color:#E6F0FF; selection-color:#172B4D; gridline-color:#EEF2F7; outline:none; border-radius:10px; }
QTableView#tableView::item { padding:8px 7px; border-bottom:1px solid #F0F3F8; }
QTableView#tableView::item:selected { color:#172B4D; }
QHeaderView::section { background:#F7F9FC; color:#61718A; padding:11px 7px; border:none; border-bottom:1px solid #E5EBF3; font-weight:700; }

/* ---------- Groupe (Livraisons) ---------- */
QGroupBox { background:#FFFFFF; border:1px solid #E4EAF2; border-radius:14px; margin-top:14px; padding:16px 12px 12px 12px; font-weight:700; color:#1B3157; }
QGroupBox::title { subcontrol-origin:margin; left:14px; padding:0 5px; background:#FFFFFF; }
QGroupBox QLabel { color:#314563; font-weight:500; }

/* ---------- Connexion ---------- */
QDialog#loginDialog { background:#FFFFFF; }
QFrame#loginBrandPanel { background:#101F3C; border:none; }
QLabel#brandWord { color:white; font-size:24pt; font-weight:800; }
QLabel#brandTag { color:#B8C9E5; font-size:10pt; }
QLabel#loginTitle { color:#14294D; font-size:23pt; font-weight:800; }
QLabel#loginSub { color:#75839A; font-size:10pt; }
QLabel#loginFieldLabel { color:#314563; font-weight:650; }
QLabel#loginMark { color:white; background:#2878F0; border-radius:17px; font-size:19pt; font-weight:800; }
QLineEdit#loginUser, QLineEdit#loginPass { padding:12px; font-size:10pt; background:#F8FAFD; border:1px solid #DFE6F0; border-radius:9px; }
QPushButton#loginButton { padding:13px; font-size:10pt; border-radius:9px; }
QLabel#loginHint { color:#8A97AA; font-size:8pt; }
)";
