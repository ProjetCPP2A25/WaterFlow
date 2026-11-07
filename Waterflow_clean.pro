QT       += core gui sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Application WaterFlow : Commandes + Livraisons + Personnel
# Ordre des dépendances : Personnel (livreurs) et Commandes -> Livraisons
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    connection.cpp \
    commande.cpp \
    employe.cpp \
    personnel.cpp \
    barchart.cpp \
    livraison.cpp \
    livraisons.cpp

HEADERS += \
    mainwindow.h \
    connection.h \
    commande.h \
    employe.h \
    personnel.h \
    barchart.h \
    livraison.h \
    livraisons.h \
    style.h \
    logindialog.h

FORMS += \
    mainwindow.ui \
    personnel.ui \
    livraisons.ui

# commandes.cpp / commandes.h : ancienne version de la page Commandes, non utilisée
# (remplacée par la page construite dans mainwindow.cpp).
