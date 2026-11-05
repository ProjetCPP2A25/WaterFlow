#pragma once
#include <QWidget>
#include <QModelIndex>
#include <QRectF>
#include <QPainter>
#include "employe.h"

class QSqlQueryModel;
class FiltreEmployes;
namespace Ui { class Personnel; }

// Vue : affiche les données et gère l'interface. Aucune requête SQL ici :
// tout passe par la classe Employe (Modèle).
class Personnel : public QWidget
{
    Q_OBJECT
public:
    explicit Personnel(QWidget *parent = nullptr);
    ~Personnel();

private slots:
    void ajouterEmploye();
    void enregistrerEmploye();
    void supprimerEmploye();
    void selectionEmploye(const QModelIndex &index);
    void rechercher();
    void exporterAttestation();
    void menuClique(int row);
    void deconnecter();

private:
    void actualiser();                 // recharge le modèle (Etmp.afficher()) et rafraîchit la vue
    void configurerTable();
    void appliquerFiltres();
    void sauvegarder(bool creation);   // creation = true -> ajouter(), sinon modifier()
    void rafraichirPlanning();
    void rafraichirGraphique();
    void remplirFormulaire(int row);
    void viderFormulaire();
    bool lireFormulaire(Employe &e);   // contrôle de saisie + remplissage de l'objet
    int idSelectionne() const;
    bool employeSelectionne(Employe &e) const;
    void selectionnerParId(int id);
    void dessinerAttestation(QPainter &p, const QRectF &zone, int dpi,
                             const Employe &e, const QString &reference) const;

    Employe Etmp;                      // objet utilisé pour appeler afficher() / supprimer()
    QSqlQueryModel *m_model = nullptr; // modèle SQL (source)
    FiltreEmployes *m_filtre = nullptr;// recherche / filtre département / tri
    int m_editingId = -1;              // id de l'employé en cours de modification (-1 = nouveau)
    Ui::Personnel *ui;
};
