#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QCryptographicHash>
#include <QFrame>

// Boîte de connexion commune à toute l'application (reprise telle quelle du module Personnel).
inline QString hashPassword(const QString &s) {
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
