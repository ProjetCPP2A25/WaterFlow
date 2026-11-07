#include "connection.h"
#include <QSqlError>
#include <QDebug>

Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    m_erreur.clear();

    QSqlDatabase db = QSqlDatabase::contains(QSqlDatabase::defaultConnection)
                          ? QSqlDatabase::database()
                          : QSqlDatabase::addDatabase("QODBC");
    if (db.isOpen())
        return true;

    // Connexion directe au pilote Oracle (aucune source ODBC à créer dans Windows).
    // Schéma wf. Quand le schéma WATERFLOW sera créé (sql/01..03), remplacer par :
    //   setUserName("waterflow") / setPassword("WaterFlow2026")
    db.setDatabaseName("DRIVER={Oracle in XE};DBQ=localhost:1521/XE;");
    db.setUserName("wf");
    db.setPassword("azerty");

    if (!db.open()) {
        m_erreur = db.lastError().text();
        qDebug() << "Échec de la connexion :" << m_erreur;
        return false;
    }
    qDebug() << "Connexion Oracle établie (schéma wf).";
    return true;
}
