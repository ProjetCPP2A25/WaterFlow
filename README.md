# WaterFlow — Commandes + Personnel + Livraisons

Application Qt 6 / C++17 (qmake) connectée à Oracle XE par ODBC (pilote « Oracle in XE »).

## Modules et liens
- **Personnel** (`EMPLOYE`) : un employé au poste **Livreur** et au statut **Actif** peut faire des livraisons.
- **Commandes** (`COMMANDE`) : une commande non livrée / non annulée peut être livrée.
- **Livraisons** (`LIVRAISON`) : liée au livreur par `ID_EMPLOYE` et à la commande par la table `REGROUPE`.
  Ajouter ou modifier une livraison met à jour la commande (statut + transporteur assigné).

## Base de données
Scripts dans `sql/`, à lancer en UTF-8 (sinon les accents des statuts sont mal enregistrés) :

```bash
cd sql
NLS_LANG=.AL32UTF8 sqlplus wf/azerty@localhost:1521/XE @02_schema.sql        # crée les 8 tables (efface les existantes)
NLS_LANG=.AL32UTF8 sqlplus wf/azerty@localhost:1521/XE @03_donnees_test.sql  # données de test
```

`01_creer_utilisateur.sql` crée un schéma séparé `waterflow` (nécessite le compte SYSTEM) ;
la connexion utilisée par l'application est dans `connection.cpp`.

## Compilation
Ouvrir `Waterflow_clean.pro` dans Qt Creator (Kit Desktop Qt 6 MinGW 64-bit), ou :

```bash
qmake Waterflow_clean.pro && mingw32-make
```

Connexion à l'application : `admin` / `admin123`.

## Test d'intégration
`tests/test_integration.pro` teste le parcours Personnel → Commande → Livraison sur la base
(crée puis supprime ses propres données).
