#include "livraisons.h"
#include "ui_livraisons.h"
#include <QColor>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QStyledItemDelegate>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QSignalBlocker>

// Numéros des colonnes du modèle (voir SELECT_BASE dans livraison.cpp)
enum Colonne { ID, COMMANDE, CLIENT, LIVREUR, ZONE, DEPART, STATUT,
               VEHICULE, ADR_DEPART, ADR_LIVRAISON, ARRIVEE, ID_LIVREUR, NB_COLONNES };

static const QString FORMAT_DATE = "dd/MM/yyyy HH:mm";

static QColor couleurStatut(const QString &statut)
{
    if (statut == "En cours")  return QColor("#00B4D8");
    if (statut == "Planifiée") return QColor("#F39C12");
    if (statut == "Livrée")    return QColor("#2ECC71");
    return QColor("#E74C3C");                                     // Retardée / Échouée
}

class StatutDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (!(option->state & QStyle::State_Selected))
            option->palette.setColor(QPalette::Text, couleurStatut(index.data().toString()));
    }
};

Livraisons::Livraisons(QWidget *parent) : QWidget(parent), ui(new Ui::Livraisons)
{
    ui->setupUi(this);
    ui->tableView->setItemDelegateForColumn(STATUT, new StatutDelegate(ui->tableView));
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    viderFormulaire();
    rechargerListes();

    connect(ui->btnAjouter,    &QPushButton::clicked, this, &Livraisons::ajouter);
    connect(ui->btnModifier,   &QPushButton::clicked, this, &Livraisons::modifier);
    connect(ui->btnSupprimer,  &QPushButton::clicked, this, &Livraisons::supprimer);
    connect(ui->btnRechercher, &QPushButton::clicked, this, &Livraisons::rechercher);
    connect(ui->btnPdf,        &QPushButton::clicked, this, &Livraisons::exporterPDF);
    connect(ui->lineEditRecherche, &QLineEdit::returnPressed, this, &Livraisons::rechercher);
    connect(ui->comboFiltreStatut, &QComboBox::currentIndexChanged, this, &Livraisons::rechercher);
    connect(ui->tableView,     &QTableView::clicked,  this, &Livraisons::remplirFormulaire);
    connect(ui->comboCommande, &QComboBox::currentIndexChanged, this, &Livraisons::commandeChoisie);
}

Livraisons::~Livraisons() { delete ui; }

void Livraisons::rechargerListes()
{
    remplirCombos(ui->comboCommande->currentData().toInt(), ui->comboChauffeur->currentData().toInt());
    actualiser();
}

// Livreurs = employés actifs au poste « Livreur » ; commandes = à livrer.
void Livraisons::remplirCombos(int idCommandeEnCours, int idLivreurEnCours)
{
    const QSignalBlocker b1(ui->comboCommande), b2(ui->comboChauffeur);

    ui->comboChauffeur->clear();
    for (const ChoixListe &c : Livraison::listeLivreurs())
        ui->comboChauffeur->addItem(c.extra.isEmpty() ? c.libelle : c.libelle + "  (" + c.extra + ")", c.id);

    ui->comboCommande->clear();
    for (const ChoixListe &c : Livraison::listeCommandes(idCommandeEnCours)) {
        ui->comboCommande->addItem(c.libelle, c.id);
        ui->comboCommande->setItemData(ui->comboCommande->count() - 1, c.extra, Qt::UserRole + 1);
    }

    int i = ui->comboChauffeur->findData(idLivreurEnCours);
    ui->comboChauffeur->setCurrentIndex(i >= 0 ? i : (ui->comboChauffeur->count() ? 0 : -1));
    i = ui->comboCommande->findData(idCommandeEnCours);
    ui->comboCommande->setCurrentIndex(i >= 0 ? i : (ui->comboCommande->count() ? 0 : -1));

    ui->comboChauffeur->setPlaceholderText("Aucun livreur : ajoutez-en dans Personnel");
    ui->comboCommande->setPlaceholderText("Aucune commande à livrer");
}

// Choix d'une commande : propose son adresse de livraison
void Livraisons::commandeChoisie(int index)
{
    if (index < 0) return;
    const QString adr = ui->comboCommande->itemData(index, Qt::UserRole + 1).toString();
    if (!adr.isEmpty() && idSelectionne() < 0)
        ui->lineEditArrivee->setText(adr);
}

// Une livraison n'est possible que si Personnel et Commandes ont fourni les données
bool Livraisons::prerequisOk()
{
    if (ui->comboChauffeur->count() == 0) {
        QMessageBox::warning(this, "Livreur manquant",
            "Aucun livreur disponible.\n\nDans Personnel, ajoutez un employé au poste « "
            + Livraison::POSTE_LIVREUR + " » avec le statut « Actif ».");
        return false;
    }
    if (ui->comboCommande->count() == 0) {
        QMessageBox::warning(this, "Commande manquante",
            "Aucune commande à livrer.\n\nDans Commandes, ajoutez une commande "
            "(statut autre que « Livrée » ou « Annulée »).");
        return false;
    }
    return true;
}

void Livraisons::afficherModele(QSqlQueryModel *nouveau)
{
    QSqlQueryModel *ancien = model;
    QItemSelectionModel *ancienneSelection = ui->tableView->selectionModel();

    model = nouveau;
    model->setParent(this);
    ui->tableView->setModel(model);
    for (int c = VEHICULE; c < NB_COLONNES; ++c)
        ui->tableView->setColumnHidden(c, true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(CLIENT, QHeaderView::Stretch);

    delete ancienneSelection;
    delete ancien;
}

void Livraisons::actualiser()
{
    afficherModele(Ltmp.rechercher(ui->lineEditRecherche->text(), ui->comboFiltreStatut->currentText()));
}

void Livraisons::viderFormulaire()
{
    ui->lineEditVehicule->clear();
    ui->lineEditDepart->setText("Dépôt WaterFlow");
    ui->lineEditArrivee->clear();
    const QDateTime debut(QDate::currentDate().addDays(1), QTime(8, 0));
    ui->dateTimeEditDepart->setDateTime(debut);
    ui->dateTimeEditArrivee->setDateTime(debut.addSecs(3 * 3600));
    ui->comboStatut->setCurrentIndex(0);
    ui->tableView->clearSelection();
    ui->tableView->setCurrentIndex(QModelIndex());
}

Livraison Livraisons::livraisonDepuisFormulaire(int id) const
{
    QString livreur = ui->comboChauffeur->currentText();
    const int p = livreur.indexOf("  (");
    if (p > 0) livreur.truncate(p);                    // retire le téléphone affiché
    return Livraison(id,
                     ui->comboCommande->currentData().toInt(),
                     ui->comboChauffeur->currentData().toInt(),
                     livreur,
                     ui->lineEditVehicule->text(),
                     ui->comboZone->currentText(),
                     ui->lineEditDepart->text(),
                     ui->lineEditArrivee->text(),
                     ui->dateTimeEditDepart->dateTime(),
                     ui->dateTimeEditArrivee->dateTime(),
                     ui->comboStatut->currentText());
}

int Livraisons::idSelectionne() const
{
    if (!model) return -1;
    const QModelIndex idx = ui->tableView->currentIndex();
    if (!idx.isValid()) return -1;
    return model->data(model->index(idx.row(), ID)).toInt();
}

// ---------- AJOUTER (Create) ----------
void Livraisons::ajouter()
{
    if (!prerequisOk()) return;
    Livraison L = livraisonDepuisFormulaire(0);

    const QString erreur = L.verifier();
    if (!erreur.isEmpty()) {
        QMessageBox::warning(this, "Erreur", erreur);
        return;
    }
    if (L.ajouter()) {
        viderFormulaire();
        remplirCombos(0, 0);                          // la commande n'est plus « à livrer »
        actualiser();
        QMessageBox::information(this, "OK", QString("Livraison n° %1 ajoutée.\n"
            "La commande a été mise à jour (statut + transporteur).").arg(L.getId()));
    } else {
        QMessageBox::critical(this, "Not OK", "Ajout non effectué.\n" + L.getErreurSql());
    }
}

// ---------- MODIFIER (Update) ----------
void Livraisons::modifier()
{
    const int id = idSelectionne();
    if (id < 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez une ligne !");
        return;
    }
    Livraison L = livraisonDepuisFormulaire(id);
    const QString erreur = L.verifier();
    if (!erreur.isEmpty()) {
        QMessageBox::warning(this, "Erreur", erreur);
        return;
    }
    if (L.modifier()) {
        remplirCombos(L.getIdCommande(), L.getIdEmploye());
        actualiser();
        QMessageBox::information(this, "OK", "Modification effectuée.");
    } else {
        QMessageBox::critical(this, "Not OK", "Modification non effectuée.\n" + L.getErreurSql());
    }
}

// ---------- SUPPRIMER (Delete) ----------
void Livraisons::supprimer()
{
    const int id = idSelectionne();
    if (id < 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez une ligne !");
        return;
    }
    if (QMessageBox::question(this, "Confirmation",
            QString("Supprimer la livraison n° %1 ?\nSa commande redeviendra « En attente » si elle n'est pas livrée.").arg(id))
        != QMessageBox::Yes)
        return;

    if (Ltmp.supprimer(id)) {
        viderFormulaire();
        remplirCombos(0, 0);
        actualiser();
        QMessageBox::information(this, "OK", "Suppression effectuée.");
    } else {
        QMessageBox::critical(this, "Not OK", "Suppression non effectuée.\n" + Ltmp.getErreurSql());
    }
}

void Livraisons::rechercher()
{
    actualiser();
}

// ---------- CLIC SUR UNE LIGNE : remplit le formulaire ----------
void Livraisons::remplirFormulaire(const QModelIndex &index)
{
    const int r = index.row();
    auto v = [&](int c) { return model->data(model->index(r, c)).toString(); };

    remplirCombos(v(COMMANDE).toInt(), v(ID_LIVREUR).toInt());
    if (ui->comboChauffeur->findData(v(ID_LIVREUR).toInt()) < 0 && !v(LIVREUR).isEmpty()) {
        // livreur devenu inactif : on l'affiche quand même pour ne pas perdre l'information
        ui->comboChauffeur->addItem(v(LIVREUR) + "  (inactif)", v(ID_LIVREUR).toInt());
        ui->comboChauffeur->setCurrentIndex(ui->comboChauffeur->count() - 1);
    }
    ui->comboZone->setCurrentText(v(ZONE));
    ui->comboStatut->setCurrentText(v(STATUT));
    ui->lineEditVehicule->setText(v(VEHICULE));
    ui->lineEditDepart->setText(v(ADR_DEPART));
    ui->lineEditArrivee->setText(v(ADR_LIVRAISON));
    ui->dateTimeEditDepart->setDateTime(QDateTime::fromString(v(DEPART), FORMAT_DATE));
    ui->dateTimeEditArrivee->setDateTime(QDateTime::fromString(v(ARRIVEE), FORMAT_DATE));
}

// ---------- BORDEREAU PDF ----------
void Livraisons::exporterPDF()
{
    const int r = ui->tableView->currentIndex().row();
    if (r < 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez une livraison !");
        return;
    }
    auto t = [&](int c) { return model->data(model->index(r, c)).toString().toHtmlEscaped(); };

    QString fichier = QFileDialog::getSaveFileName(this, "Enregistrer le bordereau",
                                                   "bordereau_" + t(ID) + ".pdf", "PDF (*.pdf)");
    if (fichier.isEmpty()) return;

    QString html = "<h1 style='color:#0A4D8C'>WaterFlow - Bordereau de livraison</h1>"
                   "<p><b>N° livraison :</b> " + t(ID) + "</p>"
                   "<p><b>Commande :</b> n° " + t(COMMANDE) + " — " + t(CLIENT) + "</p>"
                   "<p><b>Livreur :</b> " + t(LIVREUR) + "</p>"
                   "<p><b>Véhicule :</b> " + t(VEHICULE) + "</p>"
                   "<p><b>Zone :</b> " + t(ZONE) + "</p>"
                   "<p><b>Adresse de départ :</b> " + t(ADR_DEPART) + "</p>"
                   "<p><b>Adresse de livraison :</b> " + t(ADR_LIVRAISON) + "</p>"
                   "<p><b>Départ prévu :</b> " + t(DEPART) + "</p>"
                   "<p><b>Livraison prévue :</b> " + t(ARRIVEE) + "</p>"
                   "<p><b>Statut :</b> " + t(STATUT) + "</p>"
                   "<br><br><p>Signature du client : ______________________</p>";

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fichier);
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "PDF", "Bordereau enregistré !");
}
