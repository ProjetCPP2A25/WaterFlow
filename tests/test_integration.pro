QT += core sql
QT -= gui
CONFIG += c++17 console
CONFIG -= app_bundle
SOURCES += test_integration.cpp ../connection.cpp ../commande.cpp ../employe.cpp ../livraison.cpp
HEADERS += ../connection.h ../commande.h ../employe.h ../livraison.h
