/* =================================== PATIENT =================================== */

INSERT INTO srv_app."Patient" 
(
  "PatID",
  "PatNam",
  "PatBirDat",
  "PatBirTim",
  "PatSex",
  "NumPatRelStu",
  "NumPatRelSer",
  "NumPatRelIma",
  "InsertDate",
  "InsertTime",
  "Owner",
  "GroupName",
  "Priv"
)
SELECT 
  trim("PatID"),
  trim("PatNam"),
  "PatBirDat",
  "PatBirTim",
  trim("PatSex"),
  "NumPatRelStu",
  "NumPatRelSer",
  "NumPatRelIma",
  "InsertDate",
  "InsertTime",
  trim("Owner"),
  trim("GroupName"),
  trim("Priv")
FROM dblink('dbname=DICOM_DB','SELECT * FROM srv_app."Patient"')
 AS 
(
  "PatID" character(64),
  "PatNam" character(64),
  "PatBirDat" integer,
  "PatBirTim" real,
  "PatSex" character(16),
  "NumPatRelStu" integer,
  "NumPatRelSer" integer,
  "NumPatRelIma" integer,
  "InsertDate" integer,
  "InsertTime" real,
  "Owner" character(16),
  "GroupName" character(16),
  "Priv" character(9),
  "InsT" timestamp with time zone,
  "UpdT" timestamp with time zone
);

/* =================================== STUDY =================================== */
INSERT INTO srv_app."Study"
(
  "StuInsUID",
  "StuID",
  "StuDat",
  "StuTim",
  "AccNum",
  "RefPhyNam",
  "StuDes",
  "PatAge",
  "PatSiz",
  "PatWei",
  "NumStuRelSer",
  "NumStuRelIma",
  "InsertDate",
  "InsertTime",
  "Owner",
  "GroupName",
  "Priv",
  "PatParent"
)
SELECT
  trim("StuInsUID"),
  trim("StuID"),
  "StuDat",
  "StuTim",
  trim("AccNum"),
  trim("RefPhyNam"),
  trim("StuDes"),
  trim("PatAge"),
  trim("PatSiz"),
  trim("PatWei"),
  "NumStuRelSer",
  "NumStuRelIma",
  "InsertDate",
  "InsertTime",
  trim("Owner"),
  trim("GroupName"),
  trim("Priv"),
  trim("PatParent")
FROM dblink('dbname=DICOM_DB','SELECT * FROM srv_app."Study"')
 AS 
(
  "StuInsUID" character(64),
  "StuID" character(16),
  "StuDat" integer,
  "StuTim" real,
  "AccNum" character(16),
  "RefPhyNam" character(64),
  "StuDes" character(64),
  "PatAge" character(4),
  "PatSiz" character(16),
  "PatWei" character(16),
  "NumStuRelSer" integer,
  "NumStuRelIma" integer,
  "InsertDate" integer,
  "InsertTime" real,
  "Owner" character(16),
  "GroupName" character(16),
  "Priv" character(9),
  "PatParent" character(64),
  "InsT" timestamp with time zone,
  "UpdT" timestamp with time zone
 );
 
 /* =================================== SERIES =================================== */
INSERT INTO srv_app."Series"
( "SerInsUID", 
  "SerNum", 
  "Mod", 
  "ProNam", 
  "SerDes", 
  "BodParExa", 
  "ViePos", 
  "NumSerRelIma", 
  "InsertDate", 
  "InsertTime", 
  "Owner", 
  "GroupName", 
  "Priv", 
  "StuParent" ) 
SELECT 
  trim("SerInsUID"),
  trim("SerNum"),
  trim("Mod"),
  trim("ProNam"),
  trim("SerDes"),
  trim("BodParExa"),
  trim("ViePos"),
  "NumSerRelIma",
  "InsertDate",
  "InsertTime",
  trim("Owner"),
  trim("GroupName"),
  trim("Priv"),
  trim("StuParent")
FROM dblink('dbname=DICOM_DB','SELECT * FROM srv_app."Series"')
 AS 
(
  "SerInsUID" character(64),
  "SerNum" character(12),
  "Mod" character(16),
  "ProNam" character(64),
  "SerDes" character(64),
  "BodParExa" character(16),
  "ViePos" character(16),
  "NumSerRelIma" integer,
  "InsertDate" integer,
  "InsertTime" real,
  "Owner" character(16),
  "GroupName" character(16),
  "Priv" character(9),
  "StuParent" character(64),
  "InsT" timestamp with time zone,
  "UpdT" timestamp with time zone
);

/* =================================== IMAGE =================================== */
INSERT INTO srv_app."Image"
 ("SOPInsUID", 
  "SOPClaUID", 
  "SamPerPix", 
  "ImaNum", 
  "PhoInt", 
  "Row", 
  "Col", 
  "BitAll", 
  "BitSto", 
  "PixRep", 
  "PatOri", 
  "InsertDate", 
  "InsertTime", 
--  "CharSet", 
  "Owner", 
  "GroupName", 
  "Priv", 
  "SerParent")
SELECT
  trim("SOPInsUID") as "SOPInsUID",
  trim("SOPClaUID") as "SOPClaUID",
  "SamPerPix",
  trim("ImaNum") as "ImaNum",
  trim("PhoInt") as "PhoInt",
  "Row",
  "Col",
  "BitAll",
  "BitSto",
  "PixRep",
  trim("PatOri") as "PatOri",
  "InsertDate",
  "InsertTime",
--  trim("CharSet") as "CharSet",
  trim("Owner") as "Owner",
  trim("GroupName") as "GroupName",
  trim("Priv") as "Priv",
  trim("SerParent") as "SerParent"
FROM dblink('dbname=DICOM_DB','SELECT * FROM srv_app."Image"')
 AS 
(
  "SOPInsUID" character(64),
  "SOPClaUID" character(64),
  "ImaNum" character(12),
  "SamPerPix" integer,
  "PhoInt" character(16),
  "Row" integer,
  "Col" integer,
  "BitAll" integer,
  "BitSto" integer,
  "PixRep" integer,
  "PatOri" character(16),
  "InsertDate" integer,
  "InsertTime" real,
  "Owner" character(16),
  "GroupName" character(16),
  "Priv" character(9),
  "SerParent" character(64),
  "InsT" timestamp with time zone,
  "UpdT" timestamp with time zone
);

/* =================================== INSTANCE =================================== */
INSERT INTO srv_app."Instance"
 ("ImageUID",
  "RespondingTitle",
  "Medium",
  "Path",
  "Size",
  "Transfer")
SELECT
  trim("ImageUID"),
  trim("RespondingTitle"),
  trim("Medium"),
  trim("Path"),
  "Size",
  trim("Transfer")
FROM dblink('dbname=DICOM_DB','SELECT * FROM srv_app."Instance"')
 AS 
(
  isrv_appg integer,
  "ImageUID" character(64),
  "RespondingTitle" character(16),
  "Medium" character(32),
  "Path" character(255),
  "Size" integer,
  "Transfer" character(64),
  "InsT" timestamp with time zone,
  "UpdT" timestamp with time zone
);