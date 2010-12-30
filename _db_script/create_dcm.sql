/* verze web 1.4 */

/* ========================================= DATABASE CZ ========================================== 
CREATE DATABASE "DICOM_DB_NEW"
  WITH OWNER = postgres
       ENCODING = 'UTF8'
       TABLESPACE = pg_default
       LC_COLLATE = 'cs_CZ.UTF-8'
       LC_CTYPE = 'cs_CZ.UTF-8'
       CONNECTION LIMIT = -1;

CREATE TRUSTED PROCEDURAL LANGUAGE 'plpgsql'
  HANDLER plpgsql_call_handler
  VALIDATOR plpgsql_validator;
*/

/* ========================================= WEB_APP ========================================== */
CREATE SCHEMA web_app AUTHORIZATION postgres;

CREATE SEQUENCE web_app."ApplicationEntity_id_AET_seq"
  INCREMENT 1
  MINVALUE 1
  MAXVALUE 9223372036854775807
  START 2
  CACHE 1;

CREATE TABLE web_app."ApplicationEntity"
(
  "id_AET" integer NOT NULL DEFAULT nextval('web_app."ApplicationEntity_id_AET_seq"'::regclass),
  "Title" character varying(16),
  "Node" character varying(64),
  "Port" integer,
  "Comment" character varying(80),
  "Organization" character varying(32),
  "Server" boolean DEFAULT false,
  "Typ" integer NOT NULL DEFAULT 2,
  CONSTRAINT "PK_id_AET" PRIMARY KEY ("id_AET"),
  CONSTRAINT "U_Title" UNIQUE ("Title")
)
WITH (
  OIDS=FALSE
);

CREATE TABLE web_app."GRP_AET"
(
  "id_GRP" integer NOT NULL,
  "id_AET" integer NOT NULL,
  CONSTRAINT "PK_GRP_AET" PRIMARY KEY ("id_AET", "id_GRP")
)
WITH (
  OIDS=FALSE
);

CREATE SEQUENCE web_app."GroupNames_id_GRP_seq"
  INCREMENT 1
  MINVALUE 1
  MAXVALUE 9223372036854775807
  START 2
  CACHE 1;

CREATE TABLE web_app."GroupNames"
(
  "id_GRP" integer NOT NULL DEFAULT nextval('web_app."GroupNames_id_GRP_seq"'::regclass),
  "GroupName" character varying(20),
  "Comment" character varying(80),
  CONSTRAINT "PK_id_GRP" PRIMARY KEY ("id_GRP")
)
WITH (
  OIDS=FALSE
);

CREATE TABLE web_app."StorageAccess"
(
  "id_AET" integer NOT NULL,
  "DB_KEY" character varying(64),
  "id_Owner" integer,
  "id_GRP" integer,
  "Access" integer DEFAULT 0,
  root character varying(255),
  "Comment" character varying(60),
  CONSTRAINT "PK_StorageAccess" PRIMARY KEY ("id_AET")
)
WITH (
  OIDS=FALSE
);

CREATE TABLE web_app."role"
(
  id serial NOT NULL,
  nazev character varying(20) NOT NULL,
  createdate integer,
  "createuser" integer,
  popis text,
  typ character(1),
  CONSTRAINT "PK_id" PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);

CREATE TABLE web_app.roleuzivatel
(
  "role" integer NOT NULL,
  uzivatel integer NOT NULL,
  CONSTRAINT "PK_role" PRIMARY KEY (role, uzivatel)
)
WITH (
  OIDS=FALSE
);

CREATE TABLE web_app.uzivatel
(
  id serial NOT NULL,
  "login" character varying(20) NOT NULL,
  jmeno character varying(20),
  prijmeni character varying(20),
  email character varying(100),
  poznamka text,
  createdate integer,
  "createuser" integer,
  heslo character(64),
  smazan integer NOT NULL DEFAULT 0,
  CONSTRAINT uzivatel_pkey PRIMARY KEY (id),
  CONSTRAINT uzivatel_login_key UNIQUE (login)
)
WITH (
  OIDS=FALSE
);

INSERT INTO web_app."role"( nazev, createdate, "createuser", popis, typ)
    VALUES ('WL_ADMINS', 1253996412, 1, 'Role slouží pro administraci WORKLISTU', 's');

INSERT INTO web_app."GroupNames"("GroupName", "Comment")
    VALUES ('ALL', 'All clients');

/* ========================================= SRV_APP ========================================== */

CREATE SCHEMA srv_app AUTHORIZATION postgres;

CREATE TABLE srv_app."Patient"
(
  "PatID" character varying(64) NOT NULL,
  "PatNam" character varying(64) NOT NULL,
  "PatBirDat" integer NOT NULL,
  "PatBirTim" real,
  "PatSex" character varying(16) NOT NULL,
  "NumPatRelStu" integer NOT NULL,
  "NumPatRelSer" integer NOT NULL,
  "NumPatRelIma" integer NOT NULL,
  "InsertDate" integer NOT NULL,
  "InsertTime" real NOT NULL,
  "Owner" character varying(16),
  "GroupName" character varying(16),
  "Priv" character varying(9),
  "InsT" timestamp with time zone DEFAULT now(),
  "UpdT" timestamp with time zone,
  CONSTRAINT "pk_Patient" PRIMARY KEY ("PatID")
)
WITH (
  OIDS=FALSE
);

CREATE TABLE srv_app."Study"
(
  "StuInsUID" character varying(64) NOT NULL,
  "StuID" character varying(16) NOT NULL,
  "StuDat" integer NOT NULL,
  "StuTim" real NOT NULL,
  "AccNum" character varying(16) NOT NULL,
  "RefPhyNam" character varying(64) NOT NULL,
  "StuDes" character varying(64),
  "PatAge" character varying(4),
  "PatSiz" character varying(16),
  "PatWei" character varying(16),
  "NumStuRelSer" integer NOT NULL,
  "NumStuRelIma" integer NOT NULL,
  "InsertDate" integer NOT NULL,
  "InsertTime" real NOT NULL,
  "Owner" character varying(16),
  "GroupName" character varying(16),
  "Priv" character varying(9),
  "PatParent" character varying(64) NOT NULL,
  "InsT" timestamp with time zone DEFAULT now(),
  "UpdT" timestamp with time zone,
  CONSTRAINT "pk_Study" PRIMARY KEY ("StuInsUID"),
  CONSTRAINT "fk_Patient" FOREIGN KEY ("PatParent")
      REFERENCES srv_app."Patient" ("PatID") MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);

CREATE TABLE srv_app."Series"
(
  "SerInsUID" character varying(64) NOT NULL,
  "SerNum" character varying(12) NOT NULL,
  "Mod" character varying(16) NOT NULL,
  "ProNam" character varying(64),
  "SerDes" character varying(64),
  "BodParExa" character varying(16),
  "ViePos" character varying(16),
  "NumSerRelIma" integer NOT NULL,
  "InsertDate" integer NOT NULL,
  "InsertTime" real NOT NULL,
  "Owner" character varying(16),
  "GroupName" character varying(16),
  "Priv" character varying(9),
  "StuParent" character varying(64) NOT NULL,
  "InsT" timestamp with time zone DEFAULT now(),
  "UpdT" timestamp with time zone,
  CONSTRAINT "pk_Series" PRIMARY KEY ("SerInsUID"),
  CONSTRAINT "fk_Study" FOREIGN KEY ("StuParent")
      REFERENCES srv_app."Study" ("StuInsUID") MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);

CREATE TABLE srv_app."Image"
(
  "SOPInsUID" character varying(64) NOT NULL,
  "SOPClaUID" character varying(64) NOT NULL,
  "ImaNum" character varying(12) NOT NULL,
  "SamPerPix" integer NOT NULL,
  "PhoInt" character varying(16) NOT NULL,
  "Row" integer NOT NULL,
  "Col" integer NOT NULL,
  "BitAll" integer NOT NULL,
  "BitSto" integer NOT NULL,
  "PixRep" integer NOT NULL,
  "PatOri" character varying(16),
  "InsertDate" integer NOT NULL,
  "InsertTime" real NOT NULL,
  "CharSet" character varying(16),
  "Owner" character varying(16),
  "GroupName" character varying(16),
  "Priv" character varying(9),
  "SerParent" character varying(64) NOT NULL,
  "InsT" timestamp with time zone DEFAULT now(),
  "UpdT" timestamp with time zone,
  CONSTRAINT "pk_Image" PRIMARY KEY ("SOPInsUID"),
  CONSTRAINT "fk_Series" FOREIGN KEY ("SerParent")
      REFERENCES srv_app."Series" ("SerInsUID") MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);


CREATE SEQUENCE srv_app."InstanceTable_isrv_appg_seq"
  INCREMENT 1
  MINVALUE 1
  MAXVALUE 9223372036854775807
  START 32
  CACHE 1;

CREATE TABLE srv_app."Instance"
(
  isrv_appg integer NOT NULL DEFAULT nextval('srv_app."InstanceTable_isrv_appg_seq"'::regclass),
  "ImageUID" character varying(64) NOT NULL,
  "RespondingTitle" character varying(16),
  "Medium" character varying(32),
  "Path" character varying(255) NOT NULL,
  "Size" integer NOT NULL,
  "Transfer" character varying(64) NOT NULL,
  "InsT" timestamp with time zone DEFAULT now(),
  "UpdT" timestamp with time zone,
  CONSTRAINT "pk_InstanceTable" PRIMARY KEY (isrv_appg),
  CONSTRAINT "fk_Image" FOREIGN KEY ("ImageUID")
      REFERENCES srv_app."Image" ("SOPInsUID") MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);

CREATE INDEX "SOPInsUID"
  ON srv_app."Instance"
  USING btree
  ("ImageUID");
  

CREATE OR REPLACE FUNCTION srv_app."get_ModsInStudy"(stu_uid character varying)
  RETURNS character varying AS
$BODY$DECLARE out_val varchar(128) DEFAULT ' ';
DECLARE aa RECORD;
BEGIN
 FOR aa IN SELECT "Mod" AS mod FROM srv_app."Series" WHERE "StuParent" = stu_uid GROUP BY "Mod" LOOP
	out_val := out_val || aa.mod || ',';
 END LOOP;
 RETURN trim(',' from trim(out_val));
EXCEPTION WHEN OTHERS THEN
 RETURN NULL;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION srv_app."get_ModsInStudy"(character varying) OWNER TO postgres;
COMMENT ON FUNCTION srv_app."get_ModsInStudy"(character varying) IS 'Vraci seznam modalit ve studii';

CREATE OR REPLACE VIEW srv_app."PatientStudyView" AS 
 SELECT p."PatNam" AS "Pat_PatNam", p."PatID" AS "Pat_PatID", p."PatBirDat" AS "Pat_PatBirDat", p."PatBirTim" AS "Pat_PatBirTim", p."PatSex" AS "Pat_PatSex", p."NumPatRelStu" AS "Pat_NumPatRelStu", p."NumPatRelSer" AS "Pat_NumPatRelSer", p."NumPatRelIma" AS "Pat_NumPatRelIma", p."InsertDate" AS "Pat_InsertDate", p."InsertTime" AS "Pat_InsertTime", p."Owner" AS "Pat_Owner", p."GroupName" AS "Pat_GroupName", p."Priv" AS "Pat_Priv", s."StuDat" AS "Stu_StuDat", s."StuTim" AS "Stu_StuTim", s."AccNum" AS "Stu_AccNum", s."StuID" AS "Stu_StuID", s."StuInsUID" AS "Stu_StuInsUID", s."RefPhyNam" AS "Stu_RefPhyNam", s."StuDes" AS "Stu_StuDes", s."PatAge" AS "Stu_PatAge", s."PatSiz" AS "Stu_PatSiz", s."PatWei" AS "Stu_PatWei", s."NumStuRelSer" AS "Stu_NumStuRelSer", s."NumStuRelIma" AS "Stu_NumStuRelIma", s."InsertDate" AS "Stu_InsertDate", s."InsertTime" AS "Stu_InsertTime", s."Owner" AS "Stu_Owner", s."GroupName" AS "Stu_GroupName", s."Priv" AS "Stu_Priv", srv_app."get_ModsInStudy"(s."StuInsUID") AS "Ser_ModsInStudy"
   FROM srv_app."Patient" p, srv_app."Study" s
  WHERE p."PatID" = s."PatParent";  

CREATE OR REPLACE VIEW srv_app."ApplicationEntity" AS 
 SELECT ae."Title", ae."Node", ae."Port", ae."Comment", ae."Organization"
   FROM web_app."ApplicationEntity" ae;

CREATE OR REPLACE VIEW srv_app."GroupNames" AS 
 SELECT gn."GroupName", aet."Title"
   FROM web_app."GroupNames" gn, web_app."ApplicationEntity" aet, web_app."GRP_AET" gae
  WHERE aet."id_AET" = gae."id_AET" AND gn."id_GRP" = gae."id_GRP";

CREATE OR REPLACE VIEW srv_app."StorageAccess" AS 
 SELECT ae1."Title", sa."DB_KEY" AS "DbKey", ae2."Title" AS "Owner", gn."GroupName", sa."Access", sa.root, sa."Comment"
   FROM web_app."ApplicationEntity" ae1, web_app."StorageAccess" sa, web_app."GroupNames" gn, web_app."ApplicationEntity" ae2
  WHERE sa."id_AET" = ae1."id_AET" AND sa."id_Owner" = ae2."id_AET" AND sa."id_GRP" = gn."id_GRP";

CREATE OR REPLACE VIEW srv_app."StorageControl" AS 
 SELECT x."RequestingTitle", x."RespondingTitle", x."Medium", x."Root", x."NextDirectory"
   FROM ( SELECT gn."Title" AS "RequestingTitle", sa."Title" AS "RespondingTitle", 'x' AS "Medium", sa.root AS "Root", '' AS "NextDirectory"
            FROM srv_app."StorageAccess" sa
            JOIN srv_app."GroupNames" gn ON sa."GroupName"::text = gn."GroupName"::text
          UNION ALL 
          SELECT sa1."Owner" AS "RequestingTitle", sa1."Title" AS "RespondingTitle", 'x' AS "Medium", sa1.root, '' AS "NextDirectory"
            FROM srv_app."StorageAccess" sa1) x
  GROUP BY x."RequestingTitle", x."RespondingTitle", x."Medium", x."Root", x."NextDirectory";

CREATE OR REPLACE VIEW srv_app."SecurityMatrix" AS 
 SELECT sma."RequestingTitle", sma."RespondingTitle"
   FROM (( SELECT sc."RequestingTitle", sc."RespondingTitle"
             FROM srv_app."StorageControl" sc
           UNION ALL 
           SELECT gn."Title" AS "RequestingTitle", sa."Title" AS "RespondingTitle"
             FROM srv_app."StorageAccess" sa
             JOIN srv_app."GroupNames" gn ON sa."GroupName"::text = gn."GroupName"::text)
           UNION ALL 
           SELECT aet1."Title" AS "RequestingTitle", aet."Title" AS "RespondingTitle"
             FROM web_app."ApplicationEntity" aet1, web_app."StorageAccess" sa, web_app."ApplicationEntity" aet
            WHERE sa."id_AET" = aet."id_AET") sma
  GROUP BY sma."RequestingTitle", sma."RespondingTitle";