-- 3) Données de test — à exécuter en tant que WATERFLOW, après 02_schema.sql :
--   set NLS_LANG=.AL32UTF8
--   sqlplus waterflow/WaterFlow2026@localhost:1521/XE @03_donnees_test.sql
-- Pour repartir de zéro : relancer 02_schema.sql puis ce script.

-- ===================================================================== PERSONNEL
INSERT INTO EMPLOYE VALUES (1, 'SAADAOUI',  'Dhia',   'Technicien', 'Production',  DATE '2024-02-01', '22 100 200', 'dhia.saadaoui@waterflow.tn', 'Matin',      'Actif');
INSERT INTO EMPLOYE VALUES (2, 'BEN MRAD',  'Ayette', 'Technicien', 'Maintenance', DATE '2023-05-12', '22 300 400', 'ayette.benmrad@waterflow.tn', 'Après-midi', 'Actif');
INSERT INTO EMPLOYE VALUES (3, 'BEN SALAH', 'Ali',    'Livreur',    'Logistique',  DATE '2024-03-01', '22 111 222', 'ali.bensalah@waterflow.tn',  'Matin',      'Actif');
INSERT INTO EMPLOYE VALUES (4, 'TRABELSI',  'Sami',   'Livreur',    'Logistique',  DATE '2025-01-15', '22 333 444', 'sami.trabelsi@waterflow.tn', 'Après-midi', 'Actif');
INSERT INTO EMPLOYE VALUES (5, 'GHARBI',    'Nizar',  'Livreur',    'Logistique',  DATE '2023-09-10', '22 555 666', 'nizar.gharbi@waterflow.tn',  'Matin',      'Actif');
-- livreur en congé : ne doit PAS apparaître dans la liste des livreurs
INSERT INTO EMPLOYE VALUES (6, 'MEJRI',     'Karim',  'Livreur',    'Logistique',  DATE '2022-06-01', '22 777 888', 'karim.mejri@waterflow.tn',   'Nuit',       'En congé');

-- ===================================================================== PRODUCTION
INSERT INTO PRODUIT VALUES (1, SYSDATE - 2, 'Ligne A', 'Pack eau 1.5L x6',    5000, 1800, 500, 'Dhia Saadaoui', 'Disponible');
INSERT INTO PRODUIT VALUES (2, SYSDATE - 1, 'Ligne B', 'Pack eau 0.5L x12',   3000,  250, 400, 'Dhia Saadaoui', 'Stock faible');
INSERT INTO PRODUIT VALUES (3, SYSDATE - 3, 'Ligne C', 'Bonbonne 19L',         800,  600, 100, 'Dhia Saadaoui', 'Disponible');

-- ===================================================================== COMMANDES
-- 1 et 2 ont déjà une livraison ; 3 et 4 sont à livrer ; 5 livrée ; 6 annulée
INSERT INTO COMMANDE VALUES (1, 'Société Aqua Tunis', SYSTIMESTAMP - 3, 'Pack eau 1.5L x6',  120, 'Tunis, Lac 2',      SYSTIMESTAMP + 1, 'En préparation', 'Ali Ben Salah', 840);
INSERT INTO COMMANDE VALUES (2, 'Hôtel Le Palace',    SYSTIMESTAMP - 2, 'Bonbonne 19L',       40, 'Sousse, Corniche',  SYSTIMESTAMP,     'Expédiée',       'Sami Trabelsi', 520);
INSERT INTO COMMANDE VALUES (3, 'Carrefour Market',   SYSTIMESTAMP - 1, 'Pack eau 0.5L x12', 350, 'Ariana, Centre',    SYSTIMESTAMP + 3, 'En attente',     NULL,            2450);
INSERT INTO COMMANDE VALUES (4, 'Café des Amis',      SYSTIMESTAMP,     'Pack eau 1.5L x6',   60, 'La Marsa, Plage',   SYSTIMESTAMP + 4, 'En attente',     NULL,            300);
INSERT INTO COMMANDE VALUES (5, 'Clinique Pasteur',   SYSTIMESTAMP - 20,'Bonbonne 19L',       25, 'Tunis, Belvédère',  SYSTIMESTAMP - 10,'Livrée',         'Nizar Gharbi',  325);
INSERT INTO COMMANDE VALUES (6, 'Restaurant El Bey',  SYSTIMESTAMP - 8, 'Pack eau 1.5L x6',   15, 'Ben Arous, Centre', SYSTIMESTAMP - 3, 'Annulée',        NULL,            105);

INSERT INTO CONTIENT VALUES (1, 1, 120);
INSERT INTO CONTIENT VALUES (2, 3,  40);
INSERT INTO CONTIENT VALUES (3, 2, 350);
INSERT INTO CONTIENT VALUES (4, 1,  60);
INSERT INTO CONTIENT VALUES (5, 3,  25);
INSERT INTO CONTIENT VALUES (6, 1,  15);

-- ===================================================================== LIVRAISONS
INSERT INTO LIVRAISON (ID_LIVRAISON, ID_EMPLOYE, TRANSPORTEUR_CHAUFFEUR, VEHICULE, ZONE_GEOGRAPHIQUE,
                       ADRESSE_DEPART, ADRESSE_LIVRAISON, DATE_DEPART_PREVUE, DATE_LIVRAISON_PREVUE, STATUT)
VALUES (1, 3, 'Ali Ben Salah', '123 TUN 4567', 'Tunis Nord', 'Dépôt WaterFlow', 'Tunis, Lac 2',
        SYSTIMESTAMP + 1, SYSTIMESTAMP + 1 + 3/24, 'Planifiée');
INSERT INTO REGROUPE VALUES (1, 1);

INSERT INTO LIVRAISON (ID_LIVRAISON, ID_EMPLOYE, TRANSPORTEUR_CHAUFFEUR, VEHICULE, ZONE_GEOGRAPHIQUE,
                       ADRESSE_DEPART, ADRESSE_LIVRAISON, DATE_DEPART_PREVUE, DATE_LIVRAISON_PREVUE, STATUT)
VALUES (2, 4, 'Sami Trabelsi', '145 TUN 8910', 'Ariana', 'Dépôt WaterFlow', 'Sousse, Corniche',
        SYSTIMESTAMP - 2/24, SYSTIMESTAMP + 2/24, 'En cours');
INSERT INTO REGROUPE VALUES (2, 2);

INSERT INTO LIVRAISON (ID_LIVRAISON, ID_EMPLOYE, TRANSPORTEUR_CHAUFFEUR, VEHICULE, ZONE_GEOGRAPHIQUE,
                       ADRESSE_DEPART, ADRESSE_LIVRAISON, DATE_DEPART_PREVUE, DATE_LIVRAISON_PREVUE,
                       DATE_LIVRAISON_REELLE, STATUT)
VALUES (3, 5, 'Nizar Gharbi', '200 TUN 1122', 'Tunis Nord', 'Dépôt WaterFlow', 'Tunis, Belvédère',
        SYSTIMESTAMP - 11, SYSTIMESTAMP - 11 + 3/24, SYSTIMESTAMP - 11 + 2/24, 'Livrée');
INSERT INTO REGROUPE VALUES (3, 5);

-- ===================================================================== ÉQUIPEMENTS
INSERT INTO EQUIPEMENT VALUES (1, 'Remplisseuse R200', 'Remplissage', 'Ligne A', 'En service',  'Ayette Ben Mrad', 30, DATE '2022-01-10');
INSERT INTO EQUIPEMENT VALUES (2, 'Camion Isuzu',      'Véhicule',    'Dépôt',   'En service',  'Ali Ben Salah',   90, DATE '2023-04-01');
INSERT INTO MANT VALUES (1, 1, 2, SYSDATE - 15, 'Préventive', 4);

COMMIT;
exit
