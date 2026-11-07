#include "personnel.h"
#include "ui_personnel.h"
#include "barchart.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QTableView>
#include <QItemSelectionModel>
#include <QDir>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include <QSqlError>

// ---------------------------------------------------------------------------
// Pastille colorée (planning)
// ---------------------------------------------------------------------------
static QWidget *pastille(const QString &texte, const QString &couleur) {
    auto *c=new QWidget; c->setStyleSheet("background:transparent;"); auto *l=new QHBoxLayout(c); l->setContentsMargins(4,4,4,4);
    auto *x=new QLabel(texte); x->setAlignment(Qt::AlignCenter); x->setStyleSheet(QString("background:%1;color:white;border-radius:4px;padding:2px 4px;font-weight:bold;font-size:8pt;").arg(couleur)); l->addWidget(x); return c;
}

// ---------------------------------------------------------------------------
// Délégué : dessine la colonne « Statut » sous forme de pastille colorée
// ---------------------------------------------------------------------------
class StatutDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        const QString s = idx.data().toString();
        const QString col = s == "Suspendu" ? "#E74C3C" : s == "En congé" ? "#F39C12" : "#2ECC71";

        QStyleOptionViewItem o(opt);
        initStyleOption(&o, idx);
        o.text.clear();                                   // fond / sélection seulement
        const QStyle *st = opt.widget ? opt.widget->style() : QApplication::style();
        st->drawControl(QStyle::CE_ItemViewItem, &o, p, opt.widget);

        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QRect r = opt.rect.adjusted(6, 6, -6, -6);
        QFont f = opt.font;
        f.setBold(true);
        f.setPointSize(8);
        p->setFont(f);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(col));
        p->drawRoundedRect(r, 4, 4);
        p->setPen(Qt::white);
        p->drawText(r, Qt::AlignCenter, s);
        p->restore();
    }
};

// ---------------------------------------------------------------------------
// Proxy : recherche (ID, nom, prénom, poste) + filtre par département
// ---------------------------------------------------------------------------
class FiltreEmployes : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void setCritere(const QString &texte, const QString &departement)
    {
        m_texte = texte.trimmed();
        m_dep = departement;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const QAbstractItemModel *m = sourceModel();
        if (!m)
            return true;
        if (!m_dep.isEmpty() && m->index(row, 4, parent).data().toString() != m_dep)
            return false;
        if (m_texte.isEmpty())
            return true;
        QString s;
        for (int c = 0; c < 4; ++c)
            s += m->index(row, c, parent).data().toString() + " ";
        return s.contains(m_texte, Qt::CaseInsensitive);
    }

private:
    QString m_texte, m_dep;
};

// ---------------------------------------------------------------------------
Personnel::Personnel(QWidget *parent):QWidget(parent),ui(new Ui::Personnel){
    ui->setupUi(this);
    ui->labelLogo->setText("◈  WaterFlow");
    ui->menuLateral->setCurrentRow(1);
    ui->menuLateral->setFocusPolicy(Qt::NoFocus);
    const QList<QStyle::StandardPixmap> menuIcons = {
        QStyle::SP_ComputerIcon, QStyle::SP_DirHomeIcon, QStyle::SP_DriveHDIcon,
        QStyle::SP_FileDialogDetailedView, QStyle::SP_ArrowRight, QStyle::SP_DialogOpenButton,
        QStyle::SP_FileDialogInfoView, QStyle::SP_MessageBoxWarning
    };
    for (int i=0; i<ui->menuLateral->count() && i<menuIcons.size(); ++i) {
        ui->menuLateral->item(i)->setIcon(QApplication::style()->standardIcon(menuIcons[i]));
        ui->menuLateral->item(i)->setSizeHint(QSize(0,44));
    }
    ui->btnAjouter->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->btnEnregistrer->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSupprimer->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    ui->btnExporter->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));

    // Vue (QTableView) <- proxy (recherche / tri) <- modèle SQL (Etmp.afficher())
    m_filtre = new FiltreEmployes(this);
    m_filtre->setDynamicSortFilter(true);
    m_filtre->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->tableEmployes->setModel(m_filtre);
    ui->tableEmployes->setItemDelegateForColumn(6, new StatutDelegate(this));
    ui->tableEmployes->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->btnAjouter,&QPushButton::clicked,this,&Personnel::ajouterEmploye);
    connect(ui->btnEnregistrer,&QPushButton::clicked,this,&Personnel::enregistrerEmploye);
    connect(ui->btnSupprimer,&QPushButton::clicked,this,&Personnel::supprimerEmploye);
    connect(ui->tableEmployes,&QTableView::clicked,this,&Personnel::selectionEmploye);
    connect(ui->lineEditRecherche,&QLineEdit::textChanged,this,&Personnel::rechercher);
    connect(ui->comboDepartement,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->comboTri,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->btnExporter,&QPushButton::clicked,this,&Personnel::exporterAttestation);
    connect(ui->menuLateral,&QListWidget::currentRowChanged,this,&Personnel::menuClique);

    actualiser();
    viderFormulaire();
}
Personnel::~Personnel(){delete ui;}

// ---------------------------------------------------------------------------
// READ : recharge le modèle depuis la base via la classe Employe
// ---------------------------------------------------------------------------
void Personnel::actualiser()
{
    QSqlQueryModel *nouveau = Etmp.afficher();
    nouveau->setParent(this);             // libéré automatiquement avec le widget
    m_filtre->setSourceModel(nouveau);
    delete m_model;                       // l'ancien modèle n'est plus utilisé
    m_model = nouveau;

    if (m_model->lastError().isValid())
        QMessageBox::warning(this, "Base de données",
                             "Lecture des employés impossible :\n" + m_model->lastError().text());

    configurerTable();
    appliquerFiltres();
    rafraichirPlanning();
    rafraichirGraphique();
    ui->labelInfo->setText(QString("Responsable RH · %1 employé(s)").arg(m_model->rowCount()));
}

void Personnel::configurerTable()
{
    auto *t = ui->tableEmployes;
    t->setColumnHidden(8, true);          // date d'embauche (formulaire)
    t->setColumnHidden(9, true);          // téléphone (formulaire)
    t->setColumnHidden(10, true);         // id_employe numérique
    t->verticalHeader()->setDefaultSectionSize(34);

    auto *h = t->horizontalHeader();
    h->setStretchLastSection(false);
    h->setSectionResizeMode(QHeaderView::ResizeToContents);
    h->setSectionResizeMode(1, QHeaderView::Stretch);
    h->setSectionResizeMode(2, QHeaderView::Stretch);
    h->setSectionResizeMode(6, QHeaderView::Fixed);
    t->setColumnWidth(6, 92);
}

void Personnel::appliquerFiltres()
{
    const QString dep = ui->comboDepartement->currentIndex() == 0
                            ? QString() : ui->comboDepartement->currentText();
    m_filtre->setCritere(ui->lineEditRecherche->text(), dep);
    const bool zVersA = ui->comboTri->currentText().contains("Z → A");
    m_filtre->sort(1, zVersA ? Qt::DescendingOrder : Qt::AscendingOrder);
}

void Personnel::rechercher(){ appliquerFiltres(); }

void Personnel::rafraichirPlanning(){
    auto*t=ui->tablePlanning;
    if(!m_model){ t->setRowCount(0); return; }
    const int n=m_model->rowCount();
    t->setRowCount(n);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    for(int r=0;r<n;++r){
        const QString nom=m_model->index(r,1).data().toString();
        const QString prenom=m_model->index(r,2).data().toString();
        const QString shift=m_model->index(r,5).data().toString();
        const QString statut=m_model->index(r,6).data().toString();
        t->setItem(r,0,new QTableWidgetItem(prenom+" "+nom));
        for(int c=1;c<=5;++c){
            const QString s=statut=="Suspendu"||statut=="En congé"?"Absent":shift;
            const QString col=s=="Matin"?"#00B4D8":s=="Après-midi"?"#0A4D8C":s=="Nuit"?"#5D6D7E":"#E74C3C";
            t->setCellWidget(r,c,pastille(s,col));
        }
    }
}

void Personnel::rafraichirGraphique(){
    while(auto*i=ui->layoutGraphique->takeAt(0)){delete i->widget();delete i;}
    const QStringList labs={"Production","Maintenance","Logistique","Qualité"};
    QList<int> vals;
    for(const auto&d:labs){
        int n=0;
        for(int r=0;m_model && r<m_model->rowCount();++r)
            if(m_model->index(r,4).data().toString()==d) n++;
        vals<<n;
    }
    auto*g=new BarChart; g->setData(labs,vals); ui->layoutGraphique->addWidget(g);
}

// ---------------------------------------------------------------------------
// Formulaire
// ---------------------------------------------------------------------------
void Personnel::viderFormulaire(){
    m_editingId=-1;
    ui->lineEditNom->clear(); ui->lineEditPrenom->clear(); ui->comboPoste->setCurrentIndex(0);
    ui->lineEditTelephone->clear(); ui->lineEditMail->clear();
    ui->dateEditEmbauche->setDate(QDate::currentDate());
    ui->comboDepartementFiche->setCurrentIndex(0);
    ui->comboShift->setCurrentIndex(0);
    ui->comboStatut->setCurrentIndex(0);
    ui->labelTitreFiche->setText("Fiche employé — Nouveau");
    ui->tableEmployes->clearSelection();
}

void Personnel::remplirFormulaire(int row){
    const QAbstractItemModel *m=ui->tableEmployes->model();
    if(!m || row<0 || row>=m->rowCount()) return;
    auto val=[&](int c){ return m->index(row,c).data(); };

    m_editingId=val(10).toInt();
    ui->lineEditNom->setText(val(1).toString());
    ui->lineEditPrenom->setText(val(2).toString());
    {   // poste inconnu de la liste (ancienne saisie libre) : on le garde affiché
        const QString poste=val(3).toString();
        if(!poste.isEmpty() && ui->comboPoste->findText(poste)<0) ui->comboPoste->addItem(poste);
        ui->comboPoste->setCurrentText(poste);
    }
    ui->comboDepartementFiche->setCurrentText(val(4).toString());
    ui->comboShift->setCurrentText(val(5).toString());
    ui->comboStatut->setCurrentText(val(6).toString());
    ui->lineEditMail->setText(val(7).toString());
    const QDate d=val(8).toDate();
    ui->dateEditEmbauche->setDate(d.isValid()?d:QDate::currentDate());
    ui->lineEditTelephone->setText(val(9).toString());
    ui->labelTitreFiche->setText("Fiche employé — "+val(0).toString());
}

void Personnel::selectionEmploye(const QModelIndex &index){ remplirFormulaire(index.row()); }

int Personnel::idSelectionne() const{
    const auto sel=ui->tableEmployes->selectionModel()->selectedRows();
    if(sel.isEmpty()) return -1;
    const int id=ui->tableEmployes->model()->index(sel.first().row(),10).data().toInt();
    return id>0?id:-1;
}

bool Personnel::employeSelectionne(Employe &e) const{
    const auto sel=ui->tableEmployes->selectionModel()->selectedRows();
    if(sel.isEmpty()) return false;
    const QAbstractItemModel *m=ui->tableEmployes->model();
    const int row=sel.first().row();
    auto val=[&](int c){ return m->index(row,c).data(); };
    e=Employe(val(10).toInt(),val(1).toString(),val(2).toString(),val(3).toString(),val(4).toString(),
              val(8).toDate(),val(9).toString(),val(7).toString(),val(5).toString(),val(6).toString());
    return e.getID()>0;
}

void Personnel::selectionnerParId(int id){
    const QAbstractItemModel *m=ui->tableEmployes->model();
    for(int r=0;r<m->rowCount();++r){
        if(m->index(r,10).data().toInt()==id){
            ui->tableEmployes->selectRow(r);
            ui->tableEmployes->scrollTo(m->index(r,0));
            remplirFormulaire(r);
            return;
        }
    }
}

// Contrôle de saisie (en C++) puis remplissage de l'objet Employe
bool Personnel::lireFormulaire(Employe &e){
    const QString nom=ui->lineEditNom->text().trimmed(), prenom=ui->lineEditPrenom->text().trimmed();
    if(nom.isEmpty()||prenom.isEmpty()){
        QMessageBox::warning(this,"Données manquantes","Le nom et le prénom sont obligatoires.");
        return false;
    }
    static const QRegularExpression reTel("^\\+?[0-9\\s.()-]+$");
    const QString tel=ui->lineEditTelephone->text().trimmed();
    if(!tel.isEmpty()){
        int chiffres=0; for(const QChar &c:tel) if(c.isDigit()) ++chiffres;
        if(!reTel.match(tel).hasMatch()||chiffres<8||chiffres>15){
            QMessageBox::warning(this,"Téléphone invalide","Le téléphone doit contenir de 8 à 15 chiffres (ex. 71 123 456 ou +216 71 123 456).");
            ui->lineEditTelephone->setFocus();
            return false;
        }
    }
    static const QRegularExpression reMail("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    const QString mail=ui->lineEditMail->text().trimmed();
    if(!mail.isEmpty() && !reMail.match(mail).hasMatch()){
        QMessageBox::warning(this,"E-mail invalide","Saisissez une adresse e-mail valide (ex. prenom.nom@societe.com).");
        ui->lineEditMail->setFocus();
        return false;
    }
    if(ui->dateEditEmbauche->date()>QDate::currentDate()){
        QMessageBox::warning(this,"Date invalide","La date d'embauche ne peut pas être dans le futur.");
        return false;
    }
    e.setNom(nom);
    e.setPrenom(prenom);
    e.setPoste(ui->comboPoste->currentText());
    e.setDepartement(ui->comboDepartementFiche->currentText());
    e.setDateEmbauche(ui->dateEditEmbauche->date());
    e.setTelephone(tel);
    e.setMail(mail);
    e.setShift(ui->comboShift->currentText());
    e.setStatut(ui->comboStatut->currentText());
    return true;
}

// ---------------------------------------------------------------------------
// CREATE / UPDATE : le bouton ne contient AUCUNE requête, il appelle la classe Employe
// ---------------------------------------------------------------------------
void Personnel::sauvegarder(bool creation){
    Employe E;                                  // instancier un objet de la classe Employe
    if(!lireFormulaire(E)) return;
    if(!creation) E.setID(m_editingId);

    if(E.existeDeja()){
        QMessageBox::warning(this,"Doublon",QString("%1 %2 existe déjà.").arg(E.getNom(),E.getPrenom()));
        return;
    }

    const bool test = creation ? E.ajouter() : E.modifier();

    if(test){
        ui->lineEditRecherche->clear();
        ui->comboDepartement->setCurrentIndex(0);
        actualiser();                           // Refresh : Etmp.afficher() -> setModel
        selectionnerParId(E.getID());
        QMessageBox::information(this,"OK",creation?"Ajout effectué.":"Modification effectuée.");
    } else {
        QMessageBox::critical(this,"Not OK",
            QString(creation?"Ajout non effectué.":"Modification non effectuée.")+"\n"+E.erreur());
    }
}

void Personnel::ajouterEmploye(){
    // Un employé est sélectionné -> on prépare une fiche vierge ; sinon on ajoute.
    if(m_editingId>=0 || (ui->lineEditNom->text().trimmed().isEmpty() && ui->lineEditPrenom->text().trimmed().isEmpty())){
        viderFormulaire();
        ui->lineEditNom->setFocus();
        return;
    }
    sauvegarder(true);
}

void Personnel::enregistrerEmploye(){
    sauvegarder(m_editingId<0);                 // modifier() si un employé est sélectionné, sinon ajouter()
}

// ---------------------------------------------------------------------------
// DELETE
// ---------------------------------------------------------------------------
void Personnel::supprimerEmploye(){
    Employe cible;
    if(!employeSelectionne(cible)){
        QMessageBox::information(this,"Sélection","Sélectionnez un employé à supprimer.");
        return;
    }
    QMessageBox box(QMessageBox::Question,"Confirmation",
        QString("Supprimer définitivement %1 %2 (%3) ?\nCette action est irréversible.").arg(cible.getPrenom(),cible.getNom(),cible.getMatricule()),
        QMessageBox::NoButton,this);
    QPushButton *oui=box.addButton("Supprimer",QMessageBox::AcceptRole);
    QPushButton *non=box.addButton("Annuler",QMessageBox::RejectRole);
    box.setDefaultButton(non);
    box.exec();
    if(box.clickedButton()!=oui) return;

    const bool test=Etmp.supprimer(cible.getID());   // appel de la méthode supprimer() via l'attribut Etmp
    if(test){
        viderFormulaire();
        actualiser();                                // Refresh
        QMessageBox::information(this,"OK","Suppression effectuée.");
    } else {
        QMessageBox::critical(this,"Not OK","Suppression non effectuée.\n"+Etmp.erreur());
    }
}

// Dessine l'attestation directement avec QPainter (aucun HTML).
void Personnel::dessinerAttestation(QPainter &p, const QRectF &zone, int dpi,
                                    const Employe &e, const QString &reference) const
{
    const double mm = dpi / 25.4;
    double y = zone.top();
    const QColor bleu("#0A4D8C"), gris("#5B6B7B"), noir("#222222");

    // Écrit un paragraphe (retour à la ligne automatique) puis descend de « apres » millimètres.
    auto bloc = [&](const QString &texte, int pt, bool gras, const QColor &couleur, int align, double apres) {
        QFont f("Arial", pt);
        f.setBold(gras);
        p.setFont(f);
        p.setPen(couleur);
        QRectF utilise;
        p.drawText(QRectF(zone.left(), y, zone.width(), zone.bottom() - y),
                   align | Qt::TextWordWrap, texte, &utilise);
        y += utilise.height() + apres * mm;
    };

    const QString nomComplet = e.getPrenom() + " " + e.getNom().toUpper();
    const QDate embauche = e.getDateEmbauche();

    bloc("WaterFlow", 20, true, bleu, Qt::AlignLeft, 1);
    bloc("Service des Ressources Humaines", 11, false, gris, Qt::AlignLeft, 3);

    p.setPen(QPen(bleu, 0.4 * mm));
    p.drawLine(QPointF(zone.left(), y), QPointF(zone.right(), y));
    y += 6 * mm;

    bloc("Réf. : " + reference, 11, false, noir, Qt::AlignRight, 18);
    bloc("ATTESTATION DE PRÉSENCE", 22, true, bleu, Qt::AlignHCenter, 20);

    bloc("Je soussigné(e), responsable des Ressources Humaines de la société WaterFlow, "
         "atteste par la présente que :", 12, false, noir, Qt::AlignLeft, 8);
    bloc(nomComplet, 16, true, noir, Qt::AlignHCenter, 8);

    QString ligne = "matricule " + e.getMatricule() + ", occupe le poste de " + e.getPoste()
                    + " au sein du département " + e.getDepartement();
    if (embauche.isValid())
        ligne += ", et fait partie de notre personnel depuis le " + embauche.toString("dd/MM/yyyy");
    ligne += ".";
    bloc(ligne, 12, false, noir, Qt::AlignLeft, 6);

    ligne = "L'intéressé(e) est actuellement en activité et présent(e) à son poste";
    if (!e.getShift().isEmpty())
        ligne += ", affecté(e) au shift " + e.getShift().toLower();
    ligne += ".";
    bloc(ligne, 12, false, noir, Qt::AlignLeft, 6);

    bloc("Cette attestation est délivrée à l'intéressé(e) pour servir et valoir ce que de droit.",
         12, false, noir, Qt::AlignLeft, 22);

    bloc("Fait à ______________________, le " + QDate::currentDate().toString("dd/MM/yyyy"),
         12, false, noir, Qt::AlignLeft, 12);

    bloc("Le Responsable des Ressources Humaines", 12, false, noir, Qt::AlignRight, 22);
    bloc("Signature et cachet", 11, false, gris, Qt::AlignRight, 0);
}

void Personnel::exporterAttestation(){
    Employe e;
    if(!employeSelectionne(e)){
        QMessageBox::information(this,"Attestation de présence","Sélectionnez d'abord un employé dans la liste pour générer son attestation de présence.");
        return;
    }
    if(e.getStatut()!="Actif"){
        QMessageBox::warning(this,"Attestation de présence",
            QString("%1 %2 est actuellement « %3 » : une attestation de présence ne peut être délivrée que pour un employé au statut « Actif ».")
                .arg(e.getPrenom(),e.getNom(),e.getStatut()));
        return;
    }

    QString idSansTiret=e.getMatricule(); idSansTiret.remove('-');
    const QString reference=QString("ATT-%1-%2").arg(idSansTiret,QDate::currentDate().toString("yyyyMMdd"));
    const QString suggestion=QDir::homePath()+"/attestation_presence_"+e.getNom().simplified().replace(' ','_')+"_"+e.getPrenom().simplified().replace(' ','_')+".pdf";
    QString chemin=QFileDialog::getSaveFileName(this,"Enregistrer l'attestation de présence",suggestion,"Documents PDF (*.pdf)");
    if(chemin.isEmpty()) return;                        // annulé
    if(!chemin.endsWith(".pdf",Qt::CaseInsensitive)) chemin+=".pdf";

    {   // vérifie qu'on peut écrire à cet endroit avant de générer le document
        QFile test(chemin);
        if(!test.open(QIODevice::WriteOnly)){
            QMessageBox::critical(this,"Attestation de présence","Impossible d'écrire dans ce fichier (est-il ouvert dans un autre programme ?) :\n"+QDir::toNativeSeparators(chemin));
            return;
        }
    }

    {
        QPdfWriter writer(chemin);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageOrientation(QPageLayout::Portrait);
        writer.setPageMargins(QMarginsF(20,20,20,20),QPageLayout::Millimeter);
        writer.setTitle("Attestation de présence - "+e.getPrenom()+" "+e.getNom());
        writer.setCreator("WaterFlow");

        QPainter painter(&writer);
        dessinerAttestation(painter,QRectF(0,0,writer.width(),writer.height()),writer.resolution(),e,reference);
        painter.end();
    }                                                   // le fichier est finalisé ici

    if(QFileInfo(chemin).size()<=0){
        QMessageBox::critical(this,"Attestation de présence","La création du PDF a échoué :\n"+QDir::toNativeSeparators(chemin));
        return;
    }

    QMessageBox box(QMessageBox::Information,"Attestation de présence",
                    QString("L'attestation de présence a été créée :\n%1").arg(QDir::toNativeSeparators(chemin)),
                    QMessageBox::NoButton,this);
    QPushButton *ouvrir=box.addButton("Ouvrir le PDF",QMessageBox::AcceptRole);
    box.addButton("Fermer",QMessageBox::RejectRole);
    box.exec();
    if(box.clickedButton()==ouvrir) QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
}

void Personnel::menuClique(int row){if(row==1)return;QStringList names={"Tableau de bord","Personnel","Équipements","Production & stocks","Commandes","Livraisons","Statistiques","Alertes"};QMessageBox::information(this,names.value(row),"Cette section est prête pour être reliée à son module.\nLa gestion du personnel reste accessible via Personnel.");ui->menuLateral->setCurrentRow(1);}
void Personnel::deconnecter(){close();}
