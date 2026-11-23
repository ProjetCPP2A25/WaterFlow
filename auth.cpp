#include "auth.h"
#include <QCryptographicHash>
#include <QList>
#include <QStringList>

namespace {

struct Compte {
    QString email;          // identifiant de connexion, en minuscules
    QString nom;            // nom affiché
    QString hash;           // SHA-256 du mot de passe (jamais le mot de passe en clair)
    bool admin;             // true = accès à toutes les sections
    QList<int> sections;    // lignes du menu autorisées (ignoré si admin)
};

// ---------------------------------------------------------------------------
//  COMPTES  —  à modifier ici uniquement.
//  Pour changer un mot de passe :  echo -n "nouveau_mdp" | sha256sum
//  puis coller le résultat dans la colonne "hash".
//
//  Mots de passe par défaut :  admin -> admin123,  dhia -> dhia123, ayette -> ayette123,
//                              ahmed -> ahmed123,  yasmine -> yasmine123, skander -> skander123
// ---------------------------------------------------------------------------
const QList<Compte> &comptes()
{
    static const QList<Compte> liste = {
        { "admin",           "Administrateur", "240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9", true,  {} },
        { "dhia@gmail.com",    "Dhia",    "5aab9effeddc3fc69b09419e7c4c1ee1a7b7ea2ef59ff5df7c24bb9ed17f6783", false, {1} },  // Personnel
        { "ayette@gmail.com",  "Ayette",  "e65f9f6c54e437ce88da067ba3a6e21996572bf7d166654193bd09f19452512e", false, {2} },  // Équipements
        { "ahmed@gmail.com",   "Ahmed",   "c5716b88f7e6d3aa4c0f0d1d170604776f9d36481ba3d99c784ef539e6166110", false, {3} },  // Production & stocks
        { "yasmine@gmail.com", "Yasmine", "1ed0e49fec8a88f00b299caab070c7576f86b1cf90a0572e412a5c5fa6712478", false, {4} },  // Commandes
        { "skander@gmail.com", "Skander", "83b643ac4e6563ca5a149f4d4b21faca997ea9fc8008b46a111388793bdc90dc", false, {5} },  // Livraisons
    };
    return liste;
}

const Compte *s_courant = nullptr;   // compte connecté (nullptr = personne)

QString sha256(const QString &s)
{
    return QString(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
}

} // namespace

bool Session::login(const QString &email, const QString &password)
{
    const QString id = email.trimmed().toLower();
    const QString h  = sha256(password);
    for (const Compte &c : comptes()) {
        if (c.email == id && c.hash == h) {
            s_courant = &c;
            return true;
        }
    }
    s_courant = nullptr;
    return false;
}

void Session::logout()            { s_courant = nullptr; }
bool Session::isLoggedIn()        { return s_courant != nullptr; }
bool Session::isAdmin()           { return s_courant && s_courant->admin; }
QString Session::nom()            { return s_courant ? s_courant->nom   : QString(); }
QString Session::email()          { return s_courant ? s_courant->email : QString(); }

bool Session::canAccess(int menuRow)
{
    if (!s_courant) return false;
    if (s_courant->admin) return true;
    return s_courant->sections.contains(menuRow);
}

QString Session::nomSection(int menuRow)
{
    static const QStringList noms = {
        "Tableau de bord", "Personnel", "Équipements", "Production & stocks",
        "Commandes", "Livraisons", "Statistiques", "Alertes"
    };
    return noms.value(menuRow);
}
