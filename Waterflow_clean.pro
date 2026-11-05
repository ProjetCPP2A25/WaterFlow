QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    connection.cpp \
    commande.cpp \
    commandes.cpp

HEADERS += \
    mainwindow.h \
    connection.h \
    commande.h \
    commandes.h \
    style.h \
    logindialog.h

FORMS += \
    mainwindow.ui