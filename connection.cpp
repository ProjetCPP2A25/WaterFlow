#include "connection.h"

Connection::Connection() {
    // We go back to QODBC since you are using a 64-bit compiler
    db = QSqlDatabase::addDatabase("QODBC", "WaterFlowDB");
}

Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    // We use the exact 64-bit source name you created
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("wf");
    db.setPassword("azerty");

    if (db.open()) {
        qDebug() << "Connexion établie avec succès!";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}