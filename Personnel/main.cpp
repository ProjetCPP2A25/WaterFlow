#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QCryptographicHash>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include "personnel.h"
#include "connection.h"

static const char *STYLE = R"(
QWidget { background:#F4F7FB; color:#172B4D; font-family:"Segoe UI"; font-size:9pt; }
QLabel { background:transparent; }
QFrame#sidebar { background:#101F3C; border:none; }
QLabel#labelLogo { color:white; font-size:17pt; font-weight:800; padding:8px 4px 18px 4px; }
QListWidget#menuLateral { background:transparent; color:#B8C5DB; border:none; font-size:10pt; outline:none; padding-top:8px; }
QListWidget#menuLateral::item { padding:12px 12px; border-radius:8px; margin:3px 0; }
QListWidget#menuLateral::item:hover { background:#1C3156; color:white; }
QListWidget#menuLateral::item:selected { background:#2878F0; color:white; font-weight:600; }
QLabel#labelTitre { color:#122344; font-size:21pt; font-weight:800; }
QFrame#frameAlerte { background:#FFF9EC; border:1px solid #F6E5BE; border-left:4px solid #E9A23B; border-radius:10px; }
QFrame#cardListe, QFrame#cardFiche, QFrame#cardPlanning, QFrame#cardGraphique { background:#FFFFFF; border:1px solid #E4EAF2; border-radius:14px; }
QLabel#labelTitreListe, QLabel#labelTitreFiche, QLabel#labelTitrePlanning, QLabel#labelTitreGraphique { color:#1B3157; font-weight:700; font-size:10pt; padding-bottom:3px; }
QPushButton { background:#2878F0; color:white; border:none; border-radius:8px; padding:9px 15px; font-weight:650; }
QPushButton:hover { background:#1765D1; }
QPushButton:pressed { background:#124FA7; }
QPushButton#btnAjouter { background:#EAF2FF; color:#2169D5; border:1px solid #D4E4FF; }
QPushButton#btnAjouter:hover { background:#DCEAFF; }
QPushButton#btnEnregistrer { background:#2878F0; }
QPushButton#btnSupprimer { background:#FFF0F0; color:#C03945; border:1px solid #F8D9DC; }
QPushButton#btnSupprimer:hover { background:#FFE1E3; }
QPushButton#btnExporter { background:#14294D; }
QPushButton#btnExporter:hover { background:#233D68; }
QLineEdit, QComboBox, QDateEdit { background:#FFFFFF; border:1px solid #DCE4EF; border-radius:8px; padding:9px 10px; selection-background-color:#2878F0; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border:1px solid #2878F0; }
QComboBox::drop-down { border:none; width:24px; }
QTableView { background:white; border:none; alternate-background-color:#F7F9FC; selection-background-color:#E6F0FF; selection-color:#172B4D; gridline-color:#EEF2F7; outline:none; }
QTableView::item { padding-left:7px; border-bottom:1px solid #F0F3F8; }
QTableView::item:selected { color:#172B4D; }
QHeaderView::section { background:#F7F9FC; color:#61718A; padding:11px 7px; border:none; border-bottom:1px solid #E5EBF3; font-weight:700; }
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

static QString hashPassword(const QString &s) {
    return QString(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
}

class LoginDialog : public QDialog {
public:
    explicit LoginDialog(QWidget *parent=nullptr) : QDialog(parent) {
        setObjectName("loginDialog");
        setWindowTitle("WaterFlow | Connexion");
        setFixedSize(820, 500);
        auto *root = new QHBoxLayout(this);
        root->setContentsMargins(0,0,0,0);
        root->setSpacing(0);

        auto *brand = new QFrame;
        brand->setObjectName("loginBrandPanel");
        brand->setMinimumWidth(320);
        auto *brandLayout = new QVBoxLayout(brand);
        brandLayout->setContentsMargins(34,34,34,34);
        brandLayout->setSpacing(16);
        auto *mark = new QLabel("W");
        mark->setObjectName("loginMark");
        mark->setAlignment(Qt::AlignCenter);
        mark->setFixedSize(58,58);
        auto *word = new QLabel("WaterFlow");
        word->setObjectName("brandWord");
        auto *tag = new QLabel("SMART OPERATIONS\nPEOPLE. PRODUCTION. PROGRESS.");
        tag->setObjectName("brandTag");
        tag->setWordWrap(true);
        auto *decor = new QLabel("✦   WORK SMARTER\n\n     Manage your team\n     with clarity and control.");
        decor->setStyleSheet("color:#E0E9F8;font-size:13pt;line-height:150%;");
        auto *bottom = new QLabel("PERSONNEL MANAGEMENT  •  v1.0");
        bottom->setStyleSheet("color:#8297B8;font-size:8pt;letter-spacing:1px;");
        brandLayout->addWidget(mark,0,Qt::AlignLeft);
        brandLayout->addSpacing(5);
        brandLayout->addWidget(word);
        brandLayout->addWidget(tag);
        brandLayout->addStretch();
        brandLayout->addWidget(decor);
        brandLayout->addStretch();
        brandLayout->addWidget(bottom);
        root->addWidget(brand,4);

        auto *right = new QWidget;
        auto *form = new QVBoxLayout(right);
        form->setContentsMargins(48,42,48,34);
        form->setSpacing(12);
        auto *welcome = new QLabel("Bienvenue");
        welcome->setObjectName("loginTitle");
        auto *sub = new QLabel("Connectez-vous à votre espace de gestion.");
        sub->setObjectName("loginSub");
        auto *userLabel = new QLabel("Nom d’utilisateur");
        userLabel->setObjectName("loginFieldLabel");
        user = new QLineEdit;
        user->setObjectName("loginUser");
        user->setPlaceholderText("Saisir votre identifiant");
        user->setClearButtonEnabled(true);
        auto *passLabel = new QLabel("Mot de passe");
        passLabel->setObjectName("loginFieldLabel");
        pass = new QLineEdit;
        pass->setObjectName("loginPass");
        pass->setPlaceholderText("Saisir votre mot de passe");
        pass->setEchoMode(QLineEdit::Password);
        auto *btn = new QPushButton("Se connecter   →");
        btn->setObjectName("loginButton");
        error = new QLabel;
        error->setStyleSheet("color:#C03945;");
        error->setWordWrap(true);
        auto *hint = new QLabel("Accès sécurisé à votre espace WaterFlow.");
        hint->setObjectName("loginHint");
        form->addStretch(1);
        form->addWidget(welcome);
        form->addWidget(sub);
        form->addSpacing(17);
        form->addWidget(userLabel);
        form->addWidget(user);
        form->addWidget(passLabel);
        form->addWidget(pass);
        form->addSpacing(6);
        form->addWidget(btn);
        form->addWidget(error);
        form->addStretch(1);
        form->addWidget(hint);
        root->addWidget(right,5);

        connect(btn,&QPushButton::clicked,this,&LoginDialog::login);
        connect(pass,&QLineEdit::returnPressed,this,&LoginDialog::login);
        QSettings settings("WaterFlow","Personnel");
        if (!settings.contains("auth/admin"))
            settings.setValue("auth/admin", hashPassword("admin123"));
    }
private:
    void login() {
        QSettings settings("WaterFlow","Personnel");
        const QString u=user->text().trimmed(), p=pass->text();
        if (u.isEmpty() || p.isEmpty()) { error->setText("Veuillez remplir les deux champs."); return; }
        if (settings.value("auth/"+u).toString() == hashPassword(p)) accept();
        else error->setText("Identifiant ou mot de passe incorrect.");
    }
    QLineEdit *user{}, *pass{};
    QLabel *error{};
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("WaterFlow");
    app.setOrganizationName("WaterFlow");
    app.setStyle("Fusion");
    app.setStyleSheet(STYLE);

    // Une seule connexion à la base (Singleton), établie avant d'ouvrir les fenêtres.
    if (!Connection::instance().createconnection()) {
        QMessageBox::critical(nullptr, "Base de données",
            "Connexion à la base Oracle impossible :\n" + Connection::instance().lastError());
        return 1;
    }

    LoginDialog login;
    if (login.exec()!=QDialog::Accepted) return 0;
    Personnel fenetre;
    fenetre.setWindowTitle("WaterFlow | Gestion du personnel");
    fenetre.resize(1440,900);
    fenetre.show();
    return app.exec();
}
