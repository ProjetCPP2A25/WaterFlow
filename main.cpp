#include "mainwindow.h"
#include "logindialog.h"
#include "connection.h"
#include "style.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. Appliquer le style (couleurs)
    a.setStyle("Fusion");
    a.setStyleSheet(kStyleSheet);

    // 2. Connexion à la base de données
    Connection& dbConn = Connection::createInstance();
    if (!dbConn.createConnection()) {
        QMessageBox::critical(nullptr, "Base de données",
            "Connexion à la base Oracle impossible :\n" + dbConn.lastError()
            + "\n\nVérifiez qu'Oracle XE est démarré (service OracleServiceXE)\n"
              "et que le schéma WATERFLOW a été créé (scripts du dossier sql/).");
        return 1;
    }

    // 3. Afficher le Login
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    // 4. Afficher la fenêtre principale
    MainWindow w;
    w.show();
    return a.exec();
}