-- À exécuter UNE SEULE FOIS dans Oracle (SQL Developer) si la colonne n'existe pas encore.
-- (Si Oracle répond ORA-01430 « colonne déjà existante », ignorez : tout est déjà en place.)
ALTER TABLE EMPLOYE ADD (mail VARCHAR2(100));

-- Facultatif : empêcher deux employés d'avoir la même adresse e-mail.
-- ALTER TABLE EMPLOYE ADD CONSTRAINT uq_employe_mail UNIQUE (mail);

COMMIT;
