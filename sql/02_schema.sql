-- 2) Tables du projet WaterFlow — à exécuter en tant que WATERFLOW, fichier en UTF-8 :
--   set NLS_LANG=.AL32UTF8
--   sqlplus waterflow/WaterFlow2026@localhost:1521/XE @02_schema.sql
--
-- Modules : Personnel (EMPLOYE), Commandes (COMMANDE, CONTIENT), Livraisons (LIVRAISON, REGROUPE),
--           Production (PRODUIT), Équipements (EQUIPEMENT, MANT).
-- Le script peut être relancé : il supprime d'abord les tables existantes du schéma WATERFLOW.

BEGIN
  FOR t IN (SELECT table_name FROM user_tables
             WHERE table_name IN ('REGROUPE','CONTIENT','MANT','LIVRAISON','COMMANDE',
                                  'PRODUIT','EQUIPEMENT','EMPLOYE')) LOOP
    EXECUTE IMMEDIATE 'DROP TABLE ' || t.table_name || ' CASCADE CONSTRAINTS PURGE';
  END LOOP;
END;
/

-- ===================================================================== PERSONNEL
CREATE TABLE EMPLOYE (
  ID_EMPLOYE     NUMBER        CONSTRAINT pk_employe PRIMARY KEY,
  NOM            VARCHAR2(50)  NOT NULL,
  PRENOM         VARCHAR2(50)  NOT NULL,
  POSTE          VARCHAR2(100),                       -- 'Technicien', 'Livreur'
  DEPARTEMENT    VARCHAR2(100),
  DATE_EMBAUCHE  DATE,
  TELEPHONE      VARCHAR2(20),
  MAIL           VARCHAR2(100),
  SHIFT          VARCHAR2(30),
  STATUT         VARCHAR2(30)  DEFAULT 'Actif',
  CONSTRAINT ck_employe_statut CHECK (STATUT IN ('Actif', 'En congé', 'Suspendu')),
  CONSTRAINT ck_employe_shift  CHECK (SHIFT IN ('Matin', 'Après-midi', 'Nuit'))
);

-- ===================================================================== COMMANDES
CREATE TABLE COMMANDE (
  ID_COMMANDE            NUMBER        CONSTRAINT pk_commande PRIMARY KEY,
  CLIENT                 VARCHAR2(150) NOT NULL,
  DATE_COMMANDE          TIMESTAMP     DEFAULT CURRENT_TIMESTAMP,
  PRODUITS_COMMANDES     VARCHAR2(500),
  QUANTITE               NUMBER,
  ADRESSE_LIVRAISON      VARCHAR2(200),
  DATE_LIVRAISON_PREVUE  TIMESTAMP,
  STATUT                 VARCHAR2(30)  DEFAULT 'En attente',
  TRANSPORTEUR_ASSIGNE   VARCHAR2(100),               -- rempli par le module Livraisons
  MONTANT                NUMBER(12,3),
  CONSTRAINT ck_commande_statut  CHECK (STATUT IN ('En attente', 'En préparation', 'Expédiée', 'Livrée', 'Annulée')),
  CONSTRAINT ck_commande_qte     CHECK (QUANTITE > 0),
  CONSTRAINT ck_commande_montant CHECK (MONTANT >= 0)
);

-- ===================================================================== PRODUCTION
CREATE TABLE PRODUIT (
  ID_PRODUIT             NUMBER        CONSTRAINT pk_produit PRIMARY KEY,
  DATE_HEURE_PRODUCTION  DATE,
  LIGNE_PRODUCTION       VARCHAR2(100),
  PRODUIT                VARCHAR2(100) NOT NULL,
  QUANTITE_PRODUITE      NUMBER,
  QUANTITE_STOCK         NUMBER        DEFAULT 0,
  SEUIL_ALERTE_STOCK     NUMBER        DEFAULT 0,
  OPERATEUR_RESPONSABLE  VARCHAR2(100),
  STATUT                 VARCHAR2(30)
);

-- Produits d'une commande (association COMMANDE <-> PRODUIT)
CREATE TABLE CONTIENT (
  ID_COMMANDE        NUMBER NOT NULL,
  ID_PRODUIT         NUMBER NOT NULL,
  QUANTITE_COMMANDE  NUMBER NOT NULL,
  CONSTRAINT pk_contient PRIMARY KEY (ID_COMMANDE, ID_PRODUIT),
  CONSTRAINT fk_contient_commande FOREIGN KEY (ID_COMMANDE) REFERENCES COMMANDE (ID_COMMANDE) ON DELETE CASCADE,
  CONSTRAINT fk_contient_produit  FOREIGN KEY (ID_PRODUIT)  REFERENCES PRODUIT (ID_PRODUIT),
  CONSTRAINT ck_contient_qte CHECK (QUANTITE_COMMANDE > 0)
);

-- ===================================================================== LIVRAISONS
CREATE TABLE LIVRAISON (
  ID_LIVRAISON            NUMBER        CONSTRAINT pk_livraison PRIMARY KEY,
  ID_EMPLOYE              NUMBER        NOT NULL,     -- le livreur (module Personnel)
  TRANSPORTEUR_CHAUFFEUR  VARCHAR2(100),
  VEHICULE                VARCHAR2(100),
  ADRESSE_DEPART          VARCHAR2(200),
  ADRESSE_LIVRAISON       VARCHAR2(200),
  DATE_DEPART_PREVUE      TIMESTAMP,
  DATE_LIVRAISON_PREVUE   TIMESTAMP,
  DATE_LIVRAISON_REELLE   TIMESTAMP,
  STATUT                  VARCHAR2(20)  DEFAULT 'Planifiée',
  ZONE_GEOGRAPHIQUE       VARCHAR2(100),
  ITINERAIRE              VARCHAR2(500),
  CONSTRAINT fk_livraison_employe FOREIGN KEY (ID_EMPLOYE) REFERENCES EMPLOYE (ID_EMPLOYE),
  CONSTRAINT ck_livraison_statut  CHECK (STATUT IN ('Planifiée', 'En cours', 'Livrée', 'Retardée', 'Échouée')),
  CONSTRAINT ck_livraison_dates   CHECK (DATE_LIVRAISON_PREVUE > DATE_DEPART_PREVUE)
);

-- Commandes d'une livraison (association LIVRAISON <-> COMMANDE)
CREATE TABLE REGROUPE (
  ID_LIVRAISON  NUMBER NOT NULL,
  ID_COMMANDE   NUMBER NOT NULL,
  CONSTRAINT pk_regroupe PRIMARY KEY (ID_LIVRAISON, ID_COMMANDE),
  CONSTRAINT fk_regroupe_livraison FOREIGN KEY (ID_LIVRAISON) REFERENCES LIVRAISON (ID_LIVRAISON) ON DELETE CASCADE,
  CONSTRAINT fk_regroupe_commande  FOREIGN KEY (ID_COMMANDE)  REFERENCES COMMANDE (ID_COMMANDE)
);

-- ===================================================================== ÉQUIPEMENTS
CREATE TABLE EQUIPEMENT (
  ID_EQUIPEMENT           NUMBER        CONSTRAINT pk_equipement PRIMARY KEY,
  NOM                     VARCHAR2(100) NOT NULL,
  TYPE                    VARCHAR2(50),
  LOCALISATION            VARCHAR2(100),
  ETAT                    VARCHAR2(30),
  RESPONSABLE             VARCHAR2(100),
  FREQUENCE_MAINTENANCE   NUMBER,
  DATE_MISE_EN_SERVICE    DATE
);

-- Maintenances (EQUIPEMENT <-> EMPLOYE)
CREATE TABLE MANT (
  ID_MANT           NUMBER CONSTRAINT pk_mant PRIMARY KEY,
  ID_EQUIPEMENT     NUMBER NOT NULL,
  ID_EMPLOYE        NUMBER NOT NULL,
  DATE_MAINTENANCE  DATE,
  TYPE_MAINTENANCE  VARCHAR2(100),
  DUREE             NUMBER,
  CONSTRAINT fk_mant_equipement FOREIGN KEY (ID_EQUIPEMENT) REFERENCES EQUIPEMENT (ID_EQUIPEMENT),
  CONSTRAINT fk_mant_employe    FOREIGN KEY (ID_EMPLOYE)    REFERENCES EMPLOYE (ID_EMPLOYE)
);

exit
