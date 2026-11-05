#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_ajouter_clicked();
    void on_pushButton_modifier_clicked();
    void on_pushButton_supprimer_clicked();
    void on_pushButton_vider_clicked();
    void on_pushButton_exportCSV_clicked();
    void on_pushButton_majStatutAuto_clicked();
    void on_pushButton_trierDate_clicked();
    void on_lineEdit_recherche_textChanged(const QString &arg1);
    void on_comboBox_filtreStatut_currentIndexChanged(int index);
    void on_tableView_commandes_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *model;

    // Widgets créés dans le code
    QTableView *tableViewCommandes;
    QLineEdit *lineEdit_id;
    QLineEdit *lineEdit_client;
    QLineEdit *lineEdit_produit;
    QLineEdit *lineEdit_quantite;
    QDateEdit *dateEdit_commande;
    QLineEdit *lineEdit_adresse;
    QDateEdit *dateEdit_prevue;
    QComboBox *comboBox_statut;
    QLineEdit *lineEdit_transporteur;
    QLineEdit *lineEdit_montant;

    QPushButton *pushButton_ajouter;
    QPushButton *pushButton_modifier;
    QPushButton *pushButton_supprimer;
    QPushButton *pushButton_vider;

    QTableWidget *tableWidget_alertes;
    QLabel *label_caTotal;
    QLabel *label_statsDetails;

    QPushButton *pushButton_majStatutAuto;
    QPushButton *pushButton_exportCSV;
    QPushButton *pushButton_trierDate;

    QLineEdit *lineEdit_recherche;
    QComboBox *comboBox_filtreStatut;

    void chargerDonnees();
    void calculerStatistiques();
    void detecterCommandesInhabituelles();
};

#endif // MAINWINDOW_H