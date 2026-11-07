-- 1) Création du schéma (utilisateur Oracle) WATERFLOW — à exécuter en tant que SYSTEM.
--   set NLS_LANG=.AL32UTF8
--   sqlplus / as sysdba @01_creer_utilisateur.sql   (ou system/<mdp>@localhost:1521/XE)
-- L'ancien schéma wf n'est pas modifié.

CREATE USER waterflow IDENTIFIED BY WaterFlow2026
  DEFAULT TABLESPACE users
  TEMPORARY TABLESPACE temp
  QUOTA UNLIMITED ON users;

GRANT CREATE SESSION, CREATE TABLE, CREATE VIEW, CREATE SEQUENCE, CREATE TRIGGER, CREATE PROCEDURE TO waterflow;

exit
