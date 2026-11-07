#pragma once
#include <QWidget>
#include <QSqlQueryModel>
#include "livraison.h"

namespace Ui { class Livraisons; }

// Vue : affiche les données, gère l'interface et les interactions (AUCUNE requête SQL ici)
class Livraisons : public QWidget
{
    Q_OBJECT
public:
    explicit Livraisons(QWidget *parent = nullptr);
    ~Livraisons();

    // Recharge les livreurs (Personnel) et les commandes (Commandes) + le tableau.
    // Appelée par la fenêtre principale à chaque ouverture de la page.
    void rechargerListes();

private slots:
    void ajouter();
    void modifier();
    void supprimer();
    void rechercher();
    void exporterPDF();
    void remplirFormulaire(const QModelIndex &index);
    void commandeChoisie(int index);

private:
    Livraison livraisonDepuisFormulaire(int id) const;
    int idSelectionne() const;
    void afficherModele(QSqlQueryModel *nouveau);
    void actualiser();
    void viderFormulaire();
    void remplirCombos(int idCommandeEnCours, int idLivreurEnCours);
    bool prerequisOk();

    Ui::Livraisons *ui;
    QSqlQueryModel *model = nullptr;
    Livraison Ltmp;
};
