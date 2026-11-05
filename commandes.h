#ifndef COMMANDES_H
#define COMMANDES_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>

namespace Ui { class Commandes; }

class Commandes : public QWidget
{
    Q_OBJECT

public:
    explicit Commandes(QWidget *parent = nullptr);
    ~Commandes();

private slots:
    void on_pushButton_ajouter_clicked();
    void on_pushButton_modifier_clicked();
    void on_pushButton_supprimer_clicked();
    void on_pushButton_vider_clicked();
    void on_pushButton_trierDate_clicked();
    void on_pushButton_exportCSV_clicked();
    void on_pushButton_majStatutAuto_clicked();
    void on_tableWidget_commandes_cellClicked(int row, int column);
    void on_lineEdit_recherche_textChanged(const QString &arg1);
    void on_comboBox_filtreStatut_currentIndexChanged(int index);

private:
    Ui::Commandes *ui;
    void chargerDonnees();
    void calculerStatistiques();
    void detecterCommandesInhabituelles();
};

#endif // COMMANDES_H