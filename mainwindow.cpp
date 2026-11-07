#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include "commande.h"
#include "livraisons.h"
#include "personnel.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QGroupBox>
#include <QFrame>
#include <QIntValidator>
#include <QDoubleValidator>

// Colonnes du modèle renvoyé par Commande::afficher()
enum ColCmd { C_ID, C_CLIENT, C_PRODUITS, C_QTE, C_DATE, C_ADRESSE, C_PREVUE, C_STATUT, C_MONTANT, C_TRANSP };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1400, 900);

    Connection::createInstance().createConnection();   // déjà ouverte dans main : ne fait rien

    construirePageCommandes();

    // Page Personnel (module de Dhia) : sa barre latérale interne est masquée,
    // la navigation se fait par le menu de la fenêtre principale.
    pagePersonnel = new Personnel(ui->pagePersonnel);
    if (auto *sidebar = pagePersonnel->findChild<QFrame*>("sidebar"))
        sidebar->hide();
    auto *lp = new QVBoxLayout(ui->pagePersonnel);
    lp->setContentsMargins(0, 0, 0, 0);
    lp->addWidget(pagePersonnel);

    // Page Livraisons : dépend de Personnel (livreurs) et Commandes
    pageLivraisons = new Livraisons(ui->pageLivraisons);
    auto *ll = new QVBoxLayout(ui->pageLivraisons);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->addWidget(pageLivraisons);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::changerPage);
    ui->listWidget->setCurrentRow(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// À chaque changement de page on recharge : une livraison modifie les commandes,
// un employé ajouté dans Personnel doit apparaître dans la liste des livreurs.
void MainWindow::changerPage(int row)
{
    ui->stackedWidget->setCurrentIndex(row);
    if (row == 0) chargerDonnees();
    else if (row == 1) pageLivraisons->rechargerListes();
}

void MainWindow::construirePageCommandes()
{
    QWidget *page = ui->pageCommandes;

    QScrollArea *pageScroll = new QScrollArea();
    pageScroll->setWidgetResizable(true);
    QWidget *pageContent = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(pageContent);

    QLabel *titre = new QLabel("Gestion des Commandes");
    titre->setObjectName("labelTitre");
    mainLayout->addWidget(titre);

    // Recherche et filtre (en haut, comme dans les autres modules)
    QHBoxLayout *rechercheLayout = new QHBoxLayout();
    lineEdit_recherche = new QLineEdit();
    lineEdit_recherche->setPlaceholderText("Rechercher (ID, client, produit, adresse)...");
    comboBox_filtreStatut = new QComboBox();
    comboBox_filtreStatut->addItems({"Tous les statuts", "En attente", "En préparation", "Expédiée", "Livrée", "Annulée"});
    pushButton_trierDate = new QPushButton("Trier par Date");
    rechercheLayout->addWidget(lineEdit_recherche, 1);
    rechercheLayout->addWidget(comboBox_filtreStatut);
    rechercheLayout->addWidget(pushButton_trierDate);
    mainLayout->addLayout(rechercheLayout);

    tableViewCommandes = new QTableView();
    tableViewCommandes->setObjectName("tableView");
    tableViewCommandes->setMinimumHeight(250);
    tableViewCommandes->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewCommandes->setSelectionMode(QAbstractItemView::SingleSelection);
    tableViewCommandes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableViewCommandes->setAlternatingRowColors(true);
    tableViewCommandes->verticalHeader()->setVisible(false);
    mainLayout->addWidget(tableViewCommandes);

    // Formulaire
    QGroupBox *groupBox = new QGroupBox("Détails de la commande");
    QGridLayout *formLayout = new QGridLayout(groupBox);

    lineEdit_id = new QLineEdit();
    lineEdit_id->setReadOnly(true);
    lineEdit_id->setPlaceholderText("Généré automatiquement");
    lineEdit_client = new QLineEdit();
    lineEdit_produit = new QLineEdit();
    lineEdit_quantite = new QLineEdit();
    lineEdit_quantite->setValidator(new QIntValidator(1, 1000000, this));
    dateEdit_commande = new QDateEdit(QDate::currentDate());
    dateEdit_commande->setCalendarPopup(true);
    dateEdit_commande->setDisplayFormat("dd/MM/yyyy");
    lineEdit_adresse = new QLineEdit();
    dateEdit_prevue = new QDateEdit(QDate::currentDate().addDays(5));
    dateEdit_prevue->setCalendarPopup(true);
    dateEdit_prevue->setDisplayFormat("dd/MM/yyyy");
    comboBox_statut = new QComboBox();
    comboBox_statut->addItems({"En attente", "En préparation", "Expédiée", "Livrée", "Annulée"});
    lineEdit_transporteur = new QLineEdit();
    lineEdit_transporteur->setReadOnly(true);
    lineEdit_transporteur->setPlaceholderText("Assigné depuis le module Livraisons");
    lineEdit_montant = new QLineEdit();
    auto *valMontant = new QDoubleValidator(0, 1e9, 3, this);
    valMontant->setNotation(QDoubleValidator::StandardNotation);
    valMontant->setLocale(QLocale::c());
    lineEdit_montant->setValidator(valMontant);

    int r = 0;
    auto ligne = [&](const QString &l1, QWidget *w1, const QString &l2, QWidget *w2) {
        formLayout->addWidget(new QLabel(l1), r, 0); formLayout->addWidget(w1, r, 1);
        formLayout->addWidget(new QLabel(l2), r, 2); formLayout->addWidget(w2, r, 3);
        ++r;
    };
    ligne("ID Commande :", lineEdit_id, "Client / Société :", lineEdit_client);
    ligne("Produits :", lineEdit_produit, "Quantité :", lineEdit_quantite);
    ligne("Date commande :", dateEdit_commande, "Date prévisionnelle :", dateEdit_prevue);
    ligne("Adresse livraison :", lineEdit_adresse, "Montant (TND) :", lineEdit_montant);
    ligne("Statut :", comboBox_statut, "Transporteur :", lineEdit_transporteur);
    mainLayout->addWidget(groupBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    pushButton_ajouter = new QPushButton("Ajouter");
    pushButton_ajouter->setObjectName("btnAjouter");
    pushButton_modifier = new QPushButton("Modifier");
    pushButton_modifier->setObjectName("btnModifier");
    pushButton_supprimer = new QPushButton("Supprimer");
    pushButton_supprimer->setObjectName("btnSupprimer");
    pushButton_vider = new QPushButton("Vider");
    pushButton_exportCSV = new QPushButton("Exporter CSV");
    pushButton_exportCSV->setObjectName("btnExporter");
    pushButton_majStatutAuto = new QPushButton("Vérifier Retards");
    buttonLayout->addStretch();
    for (QPushButton *b : {pushButton_ajouter, pushButton_modifier, pushButton_supprimer, pushButton_vider,
                           pushButton_majStatutAuto, pushButton_exportCSV})
        buttonLayout->addWidget(b);
    mainLayout->addLayout(buttonLayout);

    label_caTotal = new QLabel();
    label_statsDetails = new QLabel();
    mainLayout->addWidget(label_caTotal);
    mainLayout->addWidget(label_statsDetails);

    tableWidget_alertes = new QTableWidget();
    tableWidget_alertes->setColumnCount(3);
    tableWidget_alertes->setHorizontalHeaderLabels({"ID Cmd", "Client", "Alerte"});
    tableWidget_alertes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget_alertes->verticalHeader()->setVisible(false);
    tableWidget_alertes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget_alertes->setMinimumHeight(110);
    mainLayout->addWidget(new QLabel("Alertes"));
    mainLayout->addWidget(tableWidget_alertes);

    pageScroll->setWidget(pageContent);
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(pageScroll);

    connect(pushButton_ajouter, &QPushButton::clicked, this, &MainWindow::on_pushButton_ajouter_clicked);
    connect(pushButton_modifier, &QPushButton::clicked, this, &MainWindow::on_pushButton_modifier_clicked);
    connect(pushButton_supprimer, &QPushButton::clicked, this, &MainWindow::on_pushButton_supprimer_clicked);
    connect(pushButton_vider, &QPushButton::clicked, this, &MainWindow::on_pushButton_vider_clicked);
    connect(pushButton_trierDate, &QPushButton::clicked, this, &MainWindow::on_pushButton_trierDate_clicked);
    connect(pushButton_exportCSV, &QPushButton::clicked, this, &MainWindow::on_pushButton_exportCSV_clicked);
    connect(pushButton_majStatutAuto, &QPushButton::clicked, this, &MainWindow::on_pushButton_majStatutAuto_clicked);
    connect(lineEdit_recherche, &QLineEdit::textChanged, this, &MainWindow::chargerDonnees);
    connect(comboBox_filtreStatut, &QComboBox::currentIndexChanged, this, &MainWindow::chargerDonnees);
    connect(tableViewCommandes, &QTableView::clicked, this, &MainWindow::on_tableView_commandes_clicked);
}

QString MainWindow::valeur(int row, int col) const
{
    return model ? model->index(row, col).data().toString() : QString();
}

// READ : recharge le tableau avec la recherche / le filtre en cours
void MainWindow::chargerDonnees()
{
    QSqlQueryModel *nouveau = Commande::afficher(lineEdit_recherche->text(),
                                                 comboBox_filtreStatut->currentText(), triDate);
    nouveau->setParent(this);
    QItemSelectionModel *ancienneSel = tableViewCommandes->selectionModel();
    tableViewCommandes->setModel(nouveau);
    delete ancienneSel;
    delete model;
    model = nouveau;

    auto *h = tableViewCommandes->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::ResizeToContents);
    h->setSectionResizeMode(C_CLIENT, QHeaderView::Stretch);
    h->setSectionResizeMode(C_ADRESSE, QHeaderView::Stretch);

    calculerStatistiques();
    detecterCommandesInhabituelles();
}

bool MainWindow::lireFormulaire(Commande &c, int id)
{
    const QString client = lineEdit_client->text().trimmed();
    const QString produits = lineEdit_produit->text().trimmed();
    bool okQ = false, okM = false;
    const int qte = lineEdit_quantite->text().toInt(&okQ);
    const double montant = lineEdit_montant->text().replace(',', '.').toDouble(&okM);

    if (client.isEmpty() || produits.isEmpty() || lineEdit_adresse->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Client, produits et adresse de livraison sont obligatoires.");
        return false;
    }
    if (!okQ || qte <= 0) {
        QMessageBox::warning(this, "Erreur de saisie", "La quantité doit être un entier positif.");
        return false;
    }
    if (!okM || montant < 0) {
        QMessageBox::warning(this, "Erreur de saisie", "Le montant doit être un nombre positif.");
        return false;
    }
    if (dateEdit_prevue->date() < dateEdit_commande->date()) {
        QMessageBox::warning(this, "Erreur de saisie", "La date prévisionnelle doit être après la date de commande.");
        return false;
    }
    c = Commande(id, client, produits, qte, dateEdit_commande->date(), lineEdit_adresse->text(),
                 dateEdit_prevue->date(), comboBox_statut->currentText(), montant,
                 lineEdit_transporteur->text());
    return true;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    Commande c;
    if (!lireFormulaire(c, 0)) return;
    if (c.ajouter()) {
        chargerDonnees();
        on_pushButton_vider_clicked();
        QMessageBox::information(this, "Succès", QString("Commande n° %1 ajoutée !").arg(c.getId()));
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'ajouter.\n" + c.erreur());
    }
}

void MainWindow::on_pushButton_modifier_clicked()
{
    const QModelIndex index = tableViewCommandes->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande.");
        return;
    }
    Commande c;
    if (!lireFormulaire(c, valeur(index.row(), C_ID).toInt())) return;
    if (c.modifier()) {
        chargerDonnees();
        QMessageBox::information(this, "Succès", "Commande modifiée !");
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de modifier.\n" + c.erreur());
    }
}

void MainWindow::on_pushButton_supprimer_clicked()
{
    const QModelIndex index = tableViewCommandes->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande.");
        return;
    }
    const int id = valeur(index.row(), C_ID).toInt();
    if (QMessageBox::question(this, "Confirmation", QString("Supprimer la commande n° %1 ?").arg(id),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    Commande c;
    if (c.supprimer(id)) {
        chargerDonnees();
        on_pushButton_vider_clicked();
        QMessageBox::information(this, "Supprimé", "Commande supprimée.");
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de supprimer.\n" + c.erreur());
    }
}

void MainWindow::on_pushButton_vider_clicked()
{
    lineEdit_id->clear();
    lineEdit_client->clear();
    lineEdit_produit->clear();
    lineEdit_quantite->clear();
    lineEdit_adresse->clear();
    lineEdit_montant->clear();
    lineEdit_transporteur->clear();
    dateEdit_commande->setDate(QDate::currentDate());
    dateEdit_prevue->setDate(QDate::currentDate().addDays(5));
    comboBox_statut->setCurrentIndex(0);
    tableViewCommandes->clearSelection();
}

void MainWindow::on_tableView_commandes_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    const int r = index.row();
    lineEdit_id->setText(valeur(r, C_ID));
    lineEdit_client->setText(valeur(r, C_CLIENT));
    lineEdit_produit->setText(valeur(r, C_PRODUITS));
    lineEdit_quantite->setText(valeur(r, C_QTE));
    dateEdit_commande->setDate(QDate::fromString(valeur(r, C_DATE), "yyyy-MM-dd"));
    lineEdit_adresse->setText(valeur(r, C_ADRESSE));
    dateEdit_prevue->setDate(QDate::fromString(valeur(r, C_PREVUE), "yyyy-MM-dd"));
    comboBox_statut->setCurrentText(valeur(r, C_STATUT));
    lineEdit_montant->setText(valeur(r, C_MONTANT));
    lineEdit_transporteur->setText(valeur(r, C_TRANSP));
}

void MainWindow::on_pushButton_trierDate_clicked()
{
    triDate = !triDate;
    pushButton_trierDate->setText(triDate ? "Trier par ID" : "Trier par Date");
    chargerDonnees();
}

void MainWindow::on_pushButton_exportCSV_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Exporter en CSV", "commandes.csv", "CSV Files (*.csv)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'écrire le fichier.");
        return;
    }
    QTextStream out(&file);
    out << "ID;Client;Produits;Qte;DateCmd;Adresse;DatePrev;Statut;Montant;Transporteur\n";
    for (int i = 0; i < model->rowCount(); ++i) {
        QStringList rowData;
        for (int j = 0; j <= C_TRANSP; ++j)
            rowData << valeur(i, j).replace(';', ',');
        out << rowData.join(";") << "\n";
    }
    QMessageBox::information(this, "Export Réussi", "Fichier CSV exporté !");
}

void MainWindow::on_pushButton_majStatutAuto_clicked()
{
    int retardCount = 0;
    const QDate aujourdhui = QDate::currentDate();
    for (int i = 0; i < model->rowCount(); ++i) {
        const QString statut = valeur(i, C_STATUT);
        const QDate datePrev = QDate::fromString(valeur(i, C_PREVUE), "yyyy-MM-dd");
        if (statut != "Livrée" && statut != "Annulée" && datePrev.isValid() && datePrev < aujourdhui)
            retardCount++;
    }
    QMessageBox::information(this, "Suivi", QString("%1 commande(s) active(s) en retard.").arg(retardCount));
}

void MainWindow::calculerStatistiques()
{
    double caTotal = 0.0;
    int attente = 0, preparation = 0, expediee = 0, livree = 0;
    for (int i = 0; i < model->rowCount(); ++i) {
        caTotal += model->index(i, C_MONTANT).data().toDouble();
        const QString statut = valeur(i, C_STATUT);
        if (statut == "En attente") attente++;
        else if (statut == "En préparation") preparation++;
        else if (statut == "Expédiée") expediee++;
        else if (statut == "Livrée") livree++;
    }
    label_caTotal->setText(QString("Chiffre d'Affaires Total : %1 TND").arg(caTotal, 0, 'f', 2));
    label_statsDetails->setText(QString("En attente: %1 | En préparation: %2 | Expédiées: %3 | Livrées: %4")
                                    .arg(attente).arg(preparation).arg(expediee).arg(livree));
}

void MainWindow::detecterCommandesInhabituelles()
{
    tableWidget_alertes->setRowCount(0);
    for (int i = 0; i < model->rowCount(); ++i) {
        if (model->index(i, C_QTE).data().toInt() > 200) {
            const int row = tableWidget_alertes->rowCount();
            tableWidget_alertes->insertRow(row);
            tableWidget_alertes->setItem(row, 0, new QTableWidgetItem(valeur(i, C_ID)));
            tableWidget_alertes->setItem(row, 1, new QTableWidgetItem(valeur(i, C_CLIENT)));
            tableWidget_alertes->setItem(row, 2, new QTableWidgetItem("Quantité élevée (> 200 unités)"));
        }
    }
}
