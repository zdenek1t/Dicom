--
-- PostgreSQL database dump
--

-- Started on 2009-10-30 20:49:14 CET

SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- TOC entry 1809 (class 1262 OID 24666)
-- Name: CTNControl; Type: DATABASE; Schema: -; Owner: -
--

CREATE DATABASE "CTNControl" WITH TEMPLATE = template0 ENCODING = 'UTF8';


\connect "CTNControl"

SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- TOC entry 1810 (class 1262 OID 24666)
-- Dependencies: 1809
-- Name: CTNControl; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON DATABASE "CTNControl" IS '28.9.2009 - opraven WL
Pro verzi 1.4';


--
-- TOC entry 9 (class 2615 OID 49792)
-- Name: conv; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA conv;


--
-- TOC entry 3 (class 2615 OID 41388)
-- Name: dicomwl; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA dicomwl;


--
-- TOC entry 8 (class 2615 OID 49627)
-- Name: srv_app; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA srv_app;


--
-- TOC entry 1 (class 2615 OID 41194)
-- Name: web_app; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA web_app;


--
-- TOC entry 354 (class 2612 OID 41412)
-- Name: plperl; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: -
--

CREATE PROCEDURAL LANGUAGE plperl;


--
-- TOC entry 355 (class 2612 OID 41413)
-- Name: plperlu; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: -
--

CREATE PROCEDURAL LANGUAGE plperlu;


--
-- TOC entry 353 (class 2612 OID 41073)
-- Name: plpgsql; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: -
--

CREATE PROCEDURAL LANGUAGE plpgsql;


SET search_path = public, pg_catalog;

--
-- TOC entry 335 (class 1247 OID 49778)
-- Dependencies: 1384
-- Name: dblink_pkey_results; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE dblink_pkey_results AS (
	"position" integer,
	colname text
);


SET search_path = conv, pg_catalog;

--
-- TOC entry 66 (class 1255 OID 49820)
-- Dependencies: 353 9
-- Name: DataTransf(); Type: FUNCTION; Schema: conv; Owner: -
--

CREATE FUNCTION "DataTransf"() RETURNS integer
    AS $$
BEGIN
/* --------------------- PATIENT ---------------------------- */
INSERT INTO srv_app."Patient"
 ("PatID", 
  "PatNam", 
  "PatBirDat", 
  "PatBirTim", 
  "PatSex", 
  "NumPatRelStu", 
  "NumPatRelIma", 
  "NumPatRelSer", 
  "InsertDate", 
  "InsertTime", 
  "Owner", 
  "Priv", 
  "GroupName")
SELECT 
  dpl.patid, 
  dpl.patnam, 
  dpl.patbirdat, 
  dpl.patbirtim, 
  dpl.patsex, 
  dpl.numpatrelstu, 
  dpl.numpatrelima, 
  dpl.numpatrelser, 
  dpl.insertdate, 
  dpl.inserttime, 
  dpl."owner", 
  dpl.priv,
  dpl.groupname 
FROM 
  conv."DIM_PatientLevel" dpl;


/* --------------------- STUDY ----------------------------- */
INSERT INTO srv_app."Study" 
 ("StuInsUID", 
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
  "PatParent")
SELECT 
  stl.stuinsuid, 
  stl.stuid,   
  stl.studat, 
  stl.stutim, 
  stl.accnum, 
  stl.refphynam, 
  stl.studes, 
  stl.patage, 
  stl.patsiz, 
  stl.patwei, 
  stl.numsturelser, 
  stl.numsturelima, 
  stl.insertdate, 
  stl.inserttime, 
  stl."owner", 
  stl.groupname, 
  stl.priv, 
  stl.patparent
FROM 
  conv."DIM_StudyLevel" stl;


/* --------------------- SERIES ----------------------------- */
INSERT INTO srv_app."Series" 
 ("SerInsUID", 
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
  "StuParent")
SELECT 
  sel.serinsuid, 
  sel.sernum, 
  sel.mod, 
  sel.pronam, 
  sel.serdes, 
  sel.bodparexa, 
  sel.viepos, 
  sel.numserrelima, 
  sel.insertdate, 
  sel.inserttime, 
  sel."owner", 
  sel.groupname, 
  sel.priv, 
  sel.stuparent
FROM 
  conv."DIM_SeriesLevel" sel;

/* --------------------- IMAGES ----------------------------- */
INSERT INTO srv_app."Image"
 ("SOPInsUID", 
  "SOPClaUID", 
  "ImaNum", 
  "SamPerPix", 
  "PhoInt", 
  "Row", 
  "Col", 
  "BitAll", 
  "BitSto", 
  "PixRep", 
  "PatOri", 
  "InsertDate", 
  "InsertTime", 
  "Owner", 
  "GroupName", 
  "Priv", 
  "SerParent")
SELECT 
  img.sopinsuid, 
  img.sopclauid, 
  img.imanum, 
  img.samperpix, 
  img.phoint, 
  img."row", 
  img.col, 
  img.bitall, 
  img.bitsto, 
  img.pixrep, 
  img.patori, 
  img.insertdate, 
  img.inserttime, 
  img."owner", 
  img.groupname, 
  img.priv, 
  img.serparent
FROM 
  conv."DIM_ImageLevel" img;

/* --------------------- INSTANCE ----------------------------- */
INSERT INTO srv_app."Instance" 
 ("ImageUID", 
  "RespondingTitle", 
  "Medium", 
  "Path", 
  "Size", 
  "Transfer")
SELECT 
  ins.imageuid, 
  ins.respondingtitle, 
  ins.medium, 
  ins.path, 
  ins.size, 
  ins.transfer
FROM 
  conv."DIM_InstanceTable" ins;
END;$$
    LANGUAGE plpgsql;


SET search_path = dicomwl, pg_catalog;

--
-- TOC entry 27 (class 1255 OID 41426)
-- Dependencies: 353 3
-- Name: CreateFile(); Type: FUNCTION; Schema: dicomwl; Owner: -
--

CREATE FUNCTION "CreateFile"() RETURNS "trigger"
    AS $$DECLARE
PatName character(150);
vm 	record;

BEGIN
 PatName := dicomwl.dcm_name(NEW."LastName", NEW."FirstName");
 
 FOR vm IN SELECT "Title", "Modality" 
             FROM dicomwl."Modalities" mo, web_app."ApplicationEntity" ae 
            WHERE ae."id_AET" = mo."AET"
	      AND "VysMetoda" = trim(NEW."VysMetoda")
  LOOP
   PERFORM dicomwl.wlm_create_file(PatName, NEW."PatientID", NEW."AccessionNumber", NEW."PatientBirthDate", NEW."PatientSex", vm."Title", vm."Modality", NEW."VysDatum");
  END LOOP;
 return NEW;
END;$$
    LANGUAGE plpgsql;


--
-- TOC entry 25 (class 1255 OID 41427)
-- Dependencies: 353 3
-- Name: DeleteFile(); Type: FUNCTION; Schema: dicomwl; Owner: -
--

CREATE FUNCTION "DeleteFile"() RETURNS "trigger"
    AS $$
DECLARE
 vm record;
BEGIN
 FOR vm IN SELECT "AET", "Modality" FROM dicomwl."Modalities" WHERE "VysMetoda" = trim(OLD."VysMetoda")
  LOOP
   PERFORM dicomwl.wlm_del_file(vm."AET", OLD."PatientID");
  END LOOP;
 return OLD;
END;$$
    LANGUAGE plpgsql;


--
-- TOC entry 24 (class 1255 OID 41425)
-- Dependencies: 3 353
-- Name: dcm_name(character varying, character varying); Type: FUNCTION; Schema: dicomwl; Owner: -
--

CREATE FUNCTION dcm_name("LastName" character varying, "FirstName" character varying) RETURNS character varying
    AS $$BEGIN
return trim("LastName") || '^' || trim("FirstName"); 
END;$$
    LANGUAGE plpgsql;


--
-- TOC entry 26 (class 1255 OID 41434)
-- Dependencies: 355 3
-- Name: wlm_create_file(character, character, numeric, date, character, character, character, timestamp with time zone); Type: FUNCTION; Schema: dicomwl; Owner: -
--

CREATE FUNCTION wlm_create_file("PatientName" character, "PatientID" character, "AccessionNumber" numeric, "PatientBirthDate" date, "PatientSex" character, "RES_AET" character, "RES_Mod" character, "StartDateTime" timestamp with time zone) RETURNS text
    AS $_X$
use lib '/DICOM/lib/pldcm';
use DICOM;

my ($elements, $tmpl_file, $out_file);

$srv_dir = '/DICOM/WLM/';
$tmpl_file = $srv_dir . 'template.wl';
$out_dir   = $srv_dir . trim($_[5]) . '/';
$out_file  = $out_dir . trim($_[1]) . '.wl';
$lck_file  = $out_dir . 'lockfile';

  mkdir $out_dir;
  open LCK_FILE, ">>$lck_file";
  close LCK_FILE;
 
  $el = DICOM->new();
  $el->fill($tmpl_file);
  $el->editHeader("PatientName="	.trim($_[0]));
  $el->editHeader("PatientID="		.trim($_[1]));
  $el->editHeader("AccessionNumber="	.trim($_[2]));
  $el->editHeader("PatientBirthDate="	.dcm_date($_[7])); 
  $el->editHeader("PatientSex="		.trim($_[4]));
  $el->editHeader("Modality="		.trim($_[6]));
  $el->editHeader("ScheduledProcedureStepStartDate=".dcm_date($_[7]));
  $el->editHeader("ScheduledProcedureStepStartTime=".dcm_time($_[7]));
  $el->write($out_file);

  return "PatName: ".$_[0]." PatId:".$_[1]." AN:".$_[2]." PatDT:".dcm_date($_[3])." Sex:".$_[4]." Mod:".$_[6]." Date:".dcm_date($_[7])." Time:".dcm_time($_[7]);

sub trim($){
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

sub dcm_date($){
	my $str = shift;
	$str =~ /(\d*)-(\d*)-(\d*)/;
	return $1 . $2 . $3;
}

sub dcm_time($){
	my $str = shift;
	$str =~ /(\d*):(\d*):(\d*)/;
	return $1 . $2 . $3;
}
$_X$
    LANGUAGE plperlu;


--
-- TOC entry 23 (class 1255 OID 41414)
-- Dependencies: 3 355
-- Name: wlm_del_file(character, character); Type: FUNCTION; Schema: dicomwl; Owner: -
--

CREATE FUNCTION wlm_del_file("AET" character, "PatientID" character) RETURNS integer
    AS $_X$my ($srv_dir);

$srv_dir = "/DICOM/WLM/";
$file_name = $srv_dir. trim($_[0])."/".trim($_[1]).".wl";

return unlink $file_name;

sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

$_X$
    LANGUAGE plperlu;


SET search_path = public, pg_catalog;

--
-- TOC entry 46 (class 1255 OID 49769)
-- Dependencies: 6
-- Name: dblink(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink(text, text) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_record'
    LANGUAGE c STRICT;


--
-- TOC entry 47 (class 1255 OID 49770)
-- Dependencies: 6
-- Name: dblink(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink(text, text, boolean) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_record'
    LANGUAGE c STRICT;


--
-- TOC entry 48 (class 1255 OID 49771)
-- Dependencies: 6
-- Name: dblink(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink(text) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_record'
    LANGUAGE c STRICT;


--
-- TOC entry 49 (class 1255 OID 49772)
-- Dependencies: 6
-- Name: dblink(text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink(text, boolean) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_record'
    LANGUAGE c STRICT;


--
-- TOC entry 56 (class 1255 OID 49781)
-- Dependencies: 6
-- Name: dblink_build_sql_delete(text, int2vector, integer, text[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_build_sql_delete(text, int2vector, integer, text[]) RETURNS text
    AS '$libdir/dblink', 'dblink_build_sql_delete'
    LANGUAGE c STRICT;


--
-- TOC entry 55 (class 1255 OID 49780)
-- Dependencies: 6
-- Name: dblink_build_sql_insert(text, int2vector, integer, text[], text[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_build_sql_insert(text, int2vector, integer, text[], text[]) RETURNS text
    AS '$libdir/dblink', 'dblink_build_sql_insert'
    LANGUAGE c STRICT;


--
-- TOC entry 57 (class 1255 OID 49782)
-- Dependencies: 6
-- Name: dblink_build_sql_update(text, int2vector, integer, text[], text[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_build_sql_update(text, int2vector, integer, text[], text[]) RETURNS text
    AS '$libdir/dblink', 'dblink_build_sql_update'
    LANGUAGE c STRICT;


--
-- TOC entry 64 (class 1255 OID 49789)
-- Dependencies: 6
-- Name: dblink_cancel_query(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_cancel_query(text) RETURNS text
    AS '$libdir/dblink', 'dblink_cancel_query'
    LANGUAGE c STRICT;


--
-- TOC entry 42 (class 1255 OID 49765)
-- Dependencies: 6
-- Name: dblink_close(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_close(text) RETURNS text
    AS '$libdir/dblink', 'dblink_close'
    LANGUAGE c STRICT;


--
-- TOC entry 43 (class 1255 OID 49766)
-- Dependencies: 6
-- Name: dblink_close(text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_close(text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_close'
    LANGUAGE c STRICT;


--
-- TOC entry 44 (class 1255 OID 49767)
-- Dependencies: 6
-- Name: dblink_close(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_close(text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_close'
    LANGUAGE c STRICT;


--
-- TOC entry 45 (class 1255 OID 49768)
-- Dependencies: 6
-- Name: dblink_close(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_close(text, text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_close'
    LANGUAGE c STRICT;


--
-- TOC entry 28 (class 1255 OID 49751)
-- Dependencies: 6
-- Name: dblink_connect(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_connect(text) RETURNS text
    AS '$libdir/dblink', 'dblink_connect'
    LANGUAGE c STRICT;


--
-- TOC entry 29 (class 1255 OID 49752)
-- Dependencies: 6
-- Name: dblink_connect(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_connect(text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_connect'
    LANGUAGE c STRICT;


--
-- TOC entry 30 (class 1255 OID 49753)
-- Dependencies: 6
-- Name: dblink_connect_u(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_connect_u(text) RETURNS text
    AS '$libdir/dblink', 'dblink_connect'
    LANGUAGE c STRICT SECURITY DEFINER;


--
-- TOC entry 31 (class 1255 OID 49754)
-- Dependencies: 6
-- Name: dblink_connect_u(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_connect_u(text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_connect'
    LANGUAGE c STRICT SECURITY DEFINER;


--
-- TOC entry 58 (class 1255 OID 49783)
-- Dependencies: 6
-- Name: dblink_current_query(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_current_query() RETURNS text
    AS '$libdir/dblink', 'dblink_current_query'
    LANGUAGE c;


--
-- TOC entry 32 (class 1255 OID 49755)
-- Dependencies: 6
-- Name: dblink_disconnect(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_disconnect() RETURNS text
    AS '$libdir/dblink', 'dblink_disconnect'
    LANGUAGE c STRICT;


--
-- TOC entry 33 (class 1255 OID 49756)
-- Dependencies: 6
-- Name: dblink_disconnect(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_disconnect(text) RETURNS text
    AS '$libdir/dblink', 'dblink_disconnect'
    LANGUAGE c STRICT;


--
-- TOC entry 65 (class 1255 OID 49790)
-- Dependencies: 6
-- Name: dblink_error_message(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_error_message(text) RETURNS text
    AS '$libdir/dblink', 'dblink_error_message'
    LANGUAGE c STRICT;


--
-- TOC entry 50 (class 1255 OID 49773)
-- Dependencies: 6
-- Name: dblink_exec(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_exec(text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_exec'
    LANGUAGE c STRICT;


--
-- TOC entry 51 (class 1255 OID 49774)
-- Dependencies: 6
-- Name: dblink_exec(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_exec(text, text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_exec'
    LANGUAGE c STRICT;


--
-- TOC entry 52 (class 1255 OID 49775)
-- Dependencies: 6
-- Name: dblink_exec(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_exec(text) RETURNS text
    AS '$libdir/dblink', 'dblink_exec'
    LANGUAGE c STRICT;


--
-- TOC entry 53 (class 1255 OID 49776)
-- Dependencies: 6
-- Name: dblink_exec(text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_exec(text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_exec'
    LANGUAGE c STRICT;


--
-- TOC entry 38 (class 1255 OID 49761)
-- Dependencies: 6
-- Name: dblink_fetch(text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_fetch(text, integer) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_fetch'
    LANGUAGE c STRICT;


--
-- TOC entry 39 (class 1255 OID 49762)
-- Dependencies: 6
-- Name: dblink_fetch(text, integer, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_fetch(text, integer, boolean) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_fetch'
    LANGUAGE c STRICT;


--
-- TOC entry 40 (class 1255 OID 49763)
-- Dependencies: 6
-- Name: dblink_fetch(text, text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_fetch(text, text, integer) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_fetch'
    LANGUAGE c STRICT;


--
-- TOC entry 41 (class 1255 OID 49764)
-- Dependencies: 6
-- Name: dblink_fetch(text, text, integer, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_fetch(text, text, integer, boolean) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_fetch'
    LANGUAGE c STRICT;


--
-- TOC entry 63 (class 1255 OID 49788)
-- Dependencies: 6
-- Name: dblink_get_connections(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_get_connections() RETURNS text[]
    AS '$libdir/dblink', 'dblink_get_connections'
    LANGUAGE c;


--
-- TOC entry 54 (class 1255 OID 49779)
-- Dependencies: 335 6
-- Name: dblink_get_pkey(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_get_pkey(text) RETURNS SETOF dblink_pkey_results
    AS '$libdir/dblink', 'dblink_get_pkey'
    LANGUAGE c STRICT;


--
-- TOC entry 61 (class 1255 OID 49786)
-- Dependencies: 6
-- Name: dblink_get_result(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_get_result(text) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_get_result'
    LANGUAGE c STRICT;


--
-- TOC entry 62 (class 1255 OID 49787)
-- Dependencies: 6
-- Name: dblink_get_result(text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_get_result(text, boolean) RETURNS SETOF record
    AS '$libdir/dblink', 'dblink_get_result'
    LANGUAGE c STRICT;


--
-- TOC entry 60 (class 1255 OID 49785)
-- Dependencies: 6
-- Name: dblink_is_busy(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_is_busy(text) RETURNS integer
    AS '$libdir/dblink', 'dblink_is_busy'
    LANGUAGE c STRICT;


--
-- TOC entry 34 (class 1255 OID 49757)
-- Dependencies: 6
-- Name: dblink_open(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_open(text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_open'
    LANGUAGE c STRICT;


--
-- TOC entry 35 (class 1255 OID 49758)
-- Dependencies: 6
-- Name: dblink_open(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_open(text, text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_open'
    LANGUAGE c STRICT;


--
-- TOC entry 36 (class 1255 OID 49759)
-- Dependencies: 6
-- Name: dblink_open(text, text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_open(text, text, text) RETURNS text
    AS '$libdir/dblink', 'dblink_open'
    LANGUAGE c STRICT;


--
-- TOC entry 37 (class 1255 OID 49760)
-- Dependencies: 6
-- Name: dblink_open(text, text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_open(text, text, text, boolean) RETURNS text
    AS '$libdir/dblink', 'dblink_open'
    LANGUAGE c STRICT;


--
-- TOC entry 59 (class 1255 OID 49784)
-- Dependencies: 6
-- Name: dblink_send_query(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dblink_send_query(text, text) RETURNS integer
    AS '$libdir/dblink', 'dblink_send_query'
    LANGUAGE c STRICT;


SET search_path = conv, pg_catalog;

--
-- TOC entry 1385 (class 1259 OID 49793)
-- Dependencies: 1478 9
-- Name: DIM_ImageLevel; Type: VIEW; Schema: conv; Owner: -
--

CREATE VIEW "DIM_ImageLevel" AS
    SELECT se.imanum, se.sopinsuid, se.sopclauid, se.samperpix, se.phoint, se."row", se.col, se.bitall, se.bitsto, se.pixrep, se.patori, se.insertdate, se.inserttime, se."owner", se.groupname, se.priv, se.serparent FROM public.dblink('dbname=DIM'::text, '
SELECT 
  il.imanum, 
  il.sopinsuid, 
  il.sopclauid, 
  il.samperpix, 
  il.phoint, 
  il."row", 
  il.col, 
  il.bitall, 
  il.bitsto, 
  il.pixrep, 
  il.patori, 
  il.insertdate, 
  il.inserttime, 
  il."owner", 
  il.groupname, 
  il.priv, 
  il.serparent
FROM 
  public.imagelevel il'::text) se(imanum character(12), sopinsuid character(64), sopclauid character(64), samperpix integer, phoint character(16), "row" integer, col integer, bitall integer, bitsto integer, pixrep integer, patori character(16), insertdate integer, inserttime real, "owner" character(16), groupname character(16), priv character(9), serparent character(64));


--
-- TOC entry 1386 (class 1259 OID 49796)
-- Dependencies: 1479 9
-- Name: DIM_InstanceTable; Type: VIEW; Schema: conv; Owner: -
--

CREATE VIEW "DIM_InstanceTable" AS
    SELECT se.imageuid, se.respondingtitle, se.medium, se.path, se.size, se.transfer FROM public.dblink('dbname=DIM'::text, '
SELECT 
  it.imageuid, 
  it.respondingtitle, 
  it.medium, 
  it.path, 
  it.size, 
  it.transfer
FROM 
  public.instancetable it'::text) se(imageuid character(64), respondingtitle character(16), medium character(32), path character(255), size integer, transfer character(64));


--
-- TOC entry 1387 (class 1259 OID 49799)
-- Dependencies: 1480 9
-- Name: DIM_PatientLevel; Type: VIEW; Schema: conv; Owner: -
--

CREATE VIEW "DIM_PatientLevel" AS
    SELECT pa.patnam, pa.patid, pa.patbirdat, pa.patbirtim, pa.patsex, pa.numpatrelstu, pa.numpatrelser, pa.numpatrelima, pa.insertdate, pa.inserttime, pa."owner", pa.groupname, pa.priv FROM public.dblink('dbname=DIM'::text, '
SELECT 
  pl.patnam, 
  pl.patid, 
  pl.patbirdat, 
  pl.patbirtim, 
  pl.patsex, 
  pl.numpatrelstu, 
  pl.numpatrelser, 
  pl.numpatrelima, 
  pl.insertdate, 
  pl.inserttime, 
  pl."owner", 
  pl.groupname, 
  pl.priv
FROM 
  public.patientlevel pl'::text) pa(patnam character(64), patid character(64), patbirdat integer, patbirtim real, patsex character(16), numpatrelstu integer, numpatrelser integer, numpatrelima integer, insertdate integer, inserttime real, "owner" character(16), groupname character(16), priv character(9));


--
-- TOC entry 1388 (class 1259 OID 49802)
-- Dependencies: 1481 9
-- Name: DIM_SeriesLevel; Type: VIEW; Schema: conv; Owner: -
--

CREATE VIEW "DIM_SeriesLevel" AS
    SELECT se.mod, se.sernum, se.serinsuid, se.pronam, se.serdes, se.bodparexa, se.viepos, se.numserrelima, se.insertdate, se.inserttime, se."owner", se.groupname, se.priv, se.stuparent FROM public.dblink('dbname=DIM'::text, '
SELECT 
  se.mod, 
  se.sernum, 
  se.serinsuid, 
  se.pronam, 
  se.serdes, 
  se.bodparexa, 
  se.viepos, 
  se.numserrelima, 
  se.insertdate, 
  se.inserttime, 
  se."owner", 
  se.groupname, 
  se.priv, 
  se.stuparent
FROM 
  public.serieslevel se'::text) se(mod character(16), sernum character(12), serinsuid character(64), pronam character(64), serdes character(64), bodparexa character(16), viepos character(16), numserrelima integer, insertdate integer, inserttime real, "owner" character(16), groupname character(16), priv character(9), stuparent character(64));


--
-- TOC entry 1389 (class 1259 OID 49805)
-- Dependencies: 1482 9
-- Name: DIM_StudyLevel; Type: VIEW; Schema: conv; Owner: -
--

CREATE VIEW "DIM_StudyLevel" AS
    SELECT st.studat, st.stutim, st.accnum, st.stuid, st.stuinsuid, st.refphynam, st.studes, st.patage, st.patsiz, st.patwei, st.numsturelser, st.numsturelima, st.insertdate, st.inserttime, st."owner", st.groupname, st.priv, st.patparent FROM public.dblink('dbname=DIM'::text, '
SELECT 
  st.studat, 
  st.stutim, 
  st.accnum, 
  st.stuid, 
  st.stuinsuid, 
  st.refphynam, 
  st.studes, 
  st.patage, 
  st.patsiz, 
  st.patwei, 
  st.numsturelser, 
  st.numsturelima, 
  st.insertdate, 
  st.inserttime, 
  st."owner", 
  st.groupname, 
  st.priv, 
  st.patparent
FROM 
  public.studylevel st'::text) st(studat integer, stutim real, accnum character(16), stuid character(16), stuinsuid character(64), refphynam character(64), studes character(64), patage character(4), patsiz character(16), patwei character(16), numsturelser integer, numsturelima integer, insertdate integer, inserttime real, "owner" character(16), groupname character(16), priv character(9), patparent character(64));


SET search_path = dicomwl, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- TOC entry 1398 (class 1259 OID 49857)
-- Dependencies: 3
-- Name: Modalities; Type: TABLE; Schema: dicomwl; Owner: -; Tablespace: 
--

CREATE TABLE "Modalities" (
    "id_VysMetoda" integer NOT NULL,
    "id_AET" integer NOT NULL,
    "id_Modality" integer
);


--
-- TOC entry 1393 (class 1259 OID 49833)
-- Dependencies: 3
-- Name: ModalityName; Type: TABLE; Schema: dicomwl; Owner: -; Tablespace: 
--

CREATE TABLE "ModalityName" (
    "id_M" integer NOT NULL,
    "Name" character varying(20) NOT NULL
);


--
-- TOC entry 1395 (class 1259 OID 49840)
-- Dependencies: 3
-- Name: VysMetodaName; Type: TABLE; Schema: dicomwl; Owner: -; Tablespace: 
--

CREATE TABLE "VysMetodaName" (
    "id_VM" integer NOT NULL,
    "Name" character varying(20) NOT NULL
);


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1364 (class 1259 OID 41199)
-- Dependencies: 1743 1744 1
-- Name: ApplicationEntity; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE "ApplicationEntity" (
    "id_AET" integer NOT NULL,
    "Title" character varying(16),
    "Node" character varying(64),
    "Port" integer,
    "Comment" character varying(80),
    "Organization" character varying(32),
    "Server" boolean DEFAULT false,
    "Typ" integer DEFAULT 2 NOT NULL
);


SET search_path = dicomwl, pg_catalog;

--
-- TOC entry 1399 (class 1259 OID 49861)
-- Dependencies: 1485 3
-- Name: AE_Metody; Type: VIEW; Schema: dicomwl; Owner: -
--

CREATE VIEW "AE_Metody" AS
    SELECT mn."Name" AS "Modality", aet."Title", vm."Name" AS "Metoda" FROM "ModalityName" mn, "VysMetodaName" vm, "Modalities" mo, web_app."ApplicationEntity" aet WHERE (((mn."id_M" = mo."id_Modality") AND (mo."id_VysMetoda" = vm."id_VM")) AND (aet."id_AET" = mo."id_AET"));


--
-- TOC entry 1392 (class 1259 OID 49831)
-- Dependencies: 3 1393
-- Name: ModalityName_id_M_seq; Type: SEQUENCE; Schema: dicomwl; Owner: -
--

CREATE SEQUENCE "ModalityName_id_M_seq"
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1816 (class 0 OID 0)
-- Dependencies: 1392
-- Name: ModalityName_id_M_seq; Type: SEQUENCE OWNED BY; Schema: dicomwl; Owner: -
--

ALTER SEQUENCE "ModalityName_id_M_seq" OWNED BY "ModalityName"."id_M";


--
-- TOC entry 1394 (class 1259 OID 49838)
-- Dependencies: 3 1395
-- Name: VysMetodaName_id_VM_seq; Type: SEQUENCE; Schema: dicomwl; Owner: -
--

CREATE SEQUENCE "VysMetodaName_id_VM_seq"
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1817 (class 0 OID 0)
-- Dependencies: 1394
-- Name: VysMetodaName_id_VM_seq; Type: SEQUENCE OWNED BY; Schema: dicomwl; Owner: -
--

ALTER SEQUENCE "VysMetodaName_id_VM_seq" OWNED BY "VysMetodaName"."id_VM";


--
-- TOC entry 1397 (class 1259 OID 49847)
-- Dependencies: 1759 1760 3
-- Name: WorkList; Type: TABLE; Schema: dicomwl; Owner: -; Tablespace: 
--

CREATE TABLE "WorkList" (
    "idWL" integer NOT NULL,
    "PatientID" character(10),
    "PatientBirthDate" date,
    "LastName" character varying(50),
    "FirstName" character varying(50),
    "AccessionNumber" numeric,
    "PatientSex" character(1),
    "VysDatum" timestamp with time zone DEFAULT now(),
    "VysMetoda" character varying(20),
    "InsDate" timestamp with time zone DEFAULT now()
);


--
-- TOC entry 1396 (class 1259 OID 49845)
-- Dependencies: 3 1397
-- Name: WorkList_idWL_seq; Type: SEQUENCE; Schema: dicomwl; Owner: -
--

CREATE SEQUENCE "WorkList_idWL_seq"
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1818 (class 0 OID 0)
-- Dependencies: 1396
-- Name: WorkList_idWL_seq; Type: SEQUENCE OWNED BY; Schema: dicomwl; Owner: -
--

ALTER SEQUENCE "WorkList_idWL_seq" OWNED BY "WorkList"."idWL";


SET search_path = public, pg_catalog;

--
-- TOC entry 1362 (class 1259 OID 41090)
-- Dependencies: 6
-- Name: ImageCopy; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE "ImageCopy" (
    "Accession" character varying NOT NULL,
    "Source" character varying(16) NOT NULL,
    "Destination" character varying(16) NOT NULL,
    "Modality" character varying(8),
    "SOPInstanceUID" character varying NOT NULL,
    "CopyNumber" integer,
    "RetryCount" integer,
    "FailureStatus" integer
);


--
-- TOC entry 1361 (class 1259 OID 41081)
-- Dependencies: 6
-- Name: ImageForward; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE "ImageForward" (
    "SourceApplication" character varying(16) NOT NULL,
    "DestinationApplication" character varying(16) NOT NULL,
    "ForwardingApplication" character varying(16)
);


--
-- TOC entry 1360 (class 1259 OID 40970)
-- Dependencies: 6
-- Name: fisaccess; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE fisaccess (
    title character(16) NOT NULL,
    dbkey character(64) NOT NULL,
    owner character(16),
    groupname character(16),
    access integer,
    comment character(80)
);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1380 (class 1259 OID 49676)
-- Dependencies: 1474 8
-- Name: ApplicationEntity; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "ApplicationEntity" AS
    SELECT ae."Title", ae."Node", ae."Port", ae."Comment", ae."Organization" FROM web_app."ApplicationEntity" ae;


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1367 (class 1259 OID 41245)
-- Dependencies: 1
-- Name: GRP_AET; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE "GRP_AET" (
    "id_GRP" integer NOT NULL,
    "id_AET" integer NOT NULL
);


--
-- TOC entry 1368 (class 1259 OID 41249)
-- Dependencies: 1
-- Name: GroupNames_id_GRP_seq; Type: SEQUENCE; Schema: web_app; Owner: -
--

CREATE SEQUENCE "GroupNames_id_GRP_seq"
    START WITH 2
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1366 (class 1259 OID 41240)
-- Dependencies: 1746 1
-- Name: GroupNames; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE "GroupNames" (
    "id_GRP" integer DEFAULT nextval('"GroupNames_id_GRP_seq"'::regclass) NOT NULL,
    "GroupName" character varying(20),
    "Comment" character varying(80)
);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1381 (class 1259 OID 49679)
-- Dependencies: 1475 8
-- Name: GroupNames; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "GroupNames" AS
    SELECT gn."GroupName", aet."Title" FROM web_app."GroupNames" gn, web_app."ApplicationEntity" aet, web_app."GRP_AET" gae WHERE ((aet."id_AET" = gae."id_AET") AND (gn."id_GRP" = gae."id_GRP"));


--
-- TOC entry 1377 (class 1259 OID 49653)
-- Dependencies: 1753 8
-- Name: Image; Type: TABLE; Schema: srv_app; Owner: -; Tablespace: 
--

CREATE TABLE "Image" (
    "SOPInsUID" character(64) NOT NULL,
    "SOPClaUID" character(64) NOT NULL,
    "ImaNum" character(12) NOT NULL,
    "SamPerPix" integer NOT NULL,
    "PhoInt" character(16) NOT NULL,
    "Row" integer NOT NULL,
    "Col" integer NOT NULL,
    "BitAll" integer NOT NULL,
    "BitSto" integer NOT NULL,
    "PixRep" integer NOT NULL,
    "PatOri" character(16),
    "InsertDate" integer NOT NULL,
    "InsertTime" real NOT NULL,
    "Owner" character(16),
    "GroupName" character(16),
    "Priv" character(9),
    "SerParent" character(64) NOT NULL,
    "InsT" timestamp with time zone DEFAULT now(),
    "UpdT" timestamp with time zone
);


--
-- TOC entry 1379 (class 1259 OID 49665)
-- Dependencies: 1755 8
-- Name: Instance; Type: TABLE; Schema: srv_app; Owner: -; Tablespace: 
--

CREATE TABLE "Instance" (
    isrv_appg integer NOT NULL,
    "ImageUID" character(64) NOT NULL,
    "RespondingTitle" character(16),
    "Medium" character(32),
    "Path" character(255) NOT NULL,
    "Size" integer NOT NULL,
    "Transfer" character(64) NOT NULL,
    "InsT" timestamp with time zone DEFAULT now(),
    "UpdT" timestamp with time zone
);


--
-- TOC entry 1378 (class 1259 OID 49663)
-- Dependencies: 1379 8
-- Name: InstanceTable_isrv_appg_seq; Type: SEQUENCE; Schema: srv_app; Owner: -
--

CREATE SEQUENCE "InstanceTable_isrv_appg_seq"
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1823 (class 0 OID 0)
-- Dependencies: 1378
-- Name: InstanceTable_isrv_appg_seq; Type: SEQUENCE OWNED BY; Schema: srv_app; Owner: -
--

ALTER SEQUENCE "InstanceTable_isrv_appg_seq" OWNED BY "Instance".isrv_appg;


--
-- TOC entry 1374 (class 1259 OID 49628)
-- Dependencies: 1750 8
-- Name: Patient; Type: TABLE; Schema: srv_app; Owner: -; Tablespace: 
--

CREATE TABLE "Patient" (
    "PatID" character(64) NOT NULL,
    "PatNam" character(64) NOT NULL,
    "PatBirDat" integer NOT NULL,
    "PatBirTim" real,
    "PatSex" character(16) NOT NULL,
    "NumPatRelStu" integer NOT NULL,
    "NumPatRelSer" integer NOT NULL,
    "NumPatRelIma" integer NOT NULL,
    "InsertDate" integer NOT NULL,
    "InsertTime" real NOT NULL,
    "Owner" character(16),
    "GroupName" character(16),
    "Priv" character(9),
    "InsT" timestamp with time zone DEFAULT now(),
    "UpdT" timestamp with time zone
);


--
-- TOC entry 1375 (class 1259 OID 49633)
-- Dependencies: 1751 8
-- Name: Study; Type: TABLE; Schema: srv_app; Owner: -; Tablespace: 
--

CREATE TABLE "Study" (
    "StuInsUID" character(64) NOT NULL,
    "StuID" character(16) NOT NULL,
    "StuDat" integer NOT NULL,
    "StuTim" real NOT NULL,
    "AccNum" character(16) NOT NULL,
    "RefPhyNam" character(64) NOT NULL,
    "StuDes" character(64),
    "PatAge" character(4),
    "PatSiz" character(16),
    "PatWei" character(16),
    "NumStuRelSer" integer NOT NULL,
    "NumStuRelIma" integer NOT NULL,
    "InsertDate" integer NOT NULL,
    "InsertTime" real NOT NULL,
    "Owner" character(16),
    "GroupName" character(16),
    "Priv" character(9),
    "PatParent" character(64) NOT NULL,
    "InsT" timestamp with time zone DEFAULT now(),
    "UpdT" timestamp with time zone
);


--
-- TOC entry 1383 (class 1259 OID 49691)
-- Dependencies: 1477 8
-- Name: PatientStudyView; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "PatientStudyView" AS
    SELECT p."PatNam" AS "Pat_PatNam", p."PatID" AS "Pat_PatID", p."PatBirDat" AS "Pat_PatBirDat", p."PatBirTim" AS "Pat_PatBirTim", p."PatSex" AS "Pat_PatSex", p."NumPatRelStu" AS "Pat_NumPatRelStu", p."NumPatRelSer" AS "Pat_NumPatRelSer", p."NumPatRelIma" AS "Pat_NumPatRelIma", p."InsertDate" AS "Pat_InsertDate", p."InsertTime" AS "Pat_InsertTime", p."Owner" AS "Pat_Owner", p."GroupName" AS "Pat_GroupName", p."Priv" AS "Pat_Priv", s."StuDat" AS "Stu_StuDat", s."StuTim" AS "Stu_StuTim", s."AccNum" AS "Stu_AccNum", s."StuID" AS "Stu_StuID", s."StuInsUID" AS "Stu_StuInsUID", s."RefPhyNam" AS "Stu_RefPhyNam", s."StuDes" AS "Stu_StuDes", s."PatAge" AS "Stu_PatAge", s."PatSiz" AS "Stu_PatSiz", s."PatWei" AS "Stu_PatWei", s."NumStuRelSer" AS "Stu_NumStuRelSer", s."NumStuRelIma" AS "Stu_NumStuRelIma", s."InsertDate" AS "Stu_InsertDate", s."InsertTime" AS "Stu_InsertTime", s."Owner" AS "Stu_Owner", s."GroupName" AS "Stu_GroupName", s."Priv" AS "Stu_Priv", s."PatParent" AS "Stu_PatParent" FROM "Patient" p, "Study" s WHERE (p."PatID" = s."PatParent");


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1365 (class 1259 OID 41225)
-- Dependencies: 1745 1
-- Name: StorageAccess; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE "StorageAccess" (
    "id_AET" integer NOT NULL,
    "DB_KEY" character varying(64),
    "id_Owner" integer,
    "id_GRP" integer,
    "Access" integer DEFAULT 0,
    root character varying(255),
    "Comment" character varying(60)
);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1382 (class 1259 OID 49682)
-- Dependencies: 1476 8
-- Name: StorageAccess; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "StorageAccess" AS
    SELECT ae1."Title", sa."DB_KEY" AS "DbKey", ae2."Title" AS "Owner", gn."GroupName", sa."Access", sa.root, sa."Comment" FROM web_app."ApplicationEntity" ae1, web_app."StorageAccess" sa, web_app."GroupNames" gn, web_app."ApplicationEntity" ae2 WHERE (((sa."id_AET" = ae1."id_AET") AND (sa."id_Owner" = ae2."id_AET")) AND (sa."id_GRP" = gn."id_GRP"));


--
-- TOC entry 1390 (class 1259 OID 49811)
-- Dependencies: 1483 8
-- Name: StorageControl; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "StorageControl" AS
    SELECT x."RequestingTitle", x."RespondingTitle", x."Medium", x."Root", x."NextDirectory" FROM (SELECT gn."Title" AS "RequestingTitle", sa."Title" AS "RespondingTitle", 'x' AS "Medium", sa.root AS "Root", '' AS "NextDirectory" FROM ("StorageAccess" sa JOIN "GroupNames" gn ON (((sa."GroupName")::text = (gn."GroupName")::text))) UNION ALL SELECT sa1."Owner" AS "RequestingTitle", sa1."Title" AS "RespondingTitle", 'x' AS "Medium", sa1.root, '' AS "NextDirectory" FROM "StorageAccess" sa1) x GROUP BY x."RequestingTitle", x."RespondingTitle", x."Medium", x."Root", x."NextDirectory";


--
-- TOC entry 1391 (class 1259 OID 49817)
-- Dependencies: 1484 8
-- Name: SecurityMatrix; Type: VIEW; Schema: srv_app; Owner: -
--

CREATE VIEW "SecurityMatrix" AS
    SELECT sma."RequestingTitle", sma."RespondingTitle" FROM ((SELECT sc."RequestingTitle", sc."RespondingTitle" FROM "StorageControl" sc UNION ALL SELECT gn."Title" AS "RequestingTitle", sa."Title" AS "RespondingTitle" FROM ("StorageAccess" sa JOIN "GroupNames" gn ON (((sa."GroupName")::text = (gn."GroupName")::text)))) UNION ALL SELECT aet1."Title" AS "RequestingTitle", aet."Title" AS "RespondingTitle" FROM web_app."ApplicationEntity" aet1, web_app."StorageAccess" sa, web_app."ApplicationEntity" aet WHERE (sa."id_AET" = aet."id_AET")) sma GROUP BY sma."RequestingTitle", sma."RespondingTitle";


--
-- TOC entry 1376 (class 1259 OID 49643)
-- Dependencies: 1752 8
-- Name: Series; Type: TABLE; Schema: srv_app; Owner: -; Tablespace: 
--

CREATE TABLE "Series" (
    "SerInsUID" character(64) NOT NULL,
    "SerNum" character(12) NOT NULL,
    "Mod" character(16) NOT NULL,
    "ProNam" character(64),
    "SerDes" character(64),
    "BodParExa" character(16),
    "ViePos" character(16),
    "NumSerRelIma" integer NOT NULL,
    "InsertDate" integer NOT NULL,
    "InsertTime" real NOT NULL,
    "Owner" character(16),
    "GroupName" character(16),
    "Priv" character(9),
    "StuParent" character(64) NOT NULL,
    "InsT" timestamp with time zone DEFAULT now(),
    "UpdT" timestamp with time zone
);


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1363 (class 1259 OID 41197)
-- Dependencies: 1 1364
-- Name: ApplicationEntity_id_AET_seq; Type: SEQUENCE; Schema: web_app; Owner: -
--

CREATE SEQUENCE "ApplicationEntity_id_AET_seq"
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1825 (class 0 OID 0)
-- Dependencies: 1363
-- Name: ApplicationEntity_id_AET_seq; Type: SEQUENCE OWNED BY; Schema: web_app; Owner: -
--

ALTER SEQUENCE "ApplicationEntity_id_AET_seq" OWNED BY "ApplicationEntity"."id_AET";


--
-- TOC entry 1370 (class 1259 OID 41254)
-- Dependencies: 1
-- Name: role; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE role (
    id integer NOT NULL,
    nazev character varying(20) NOT NULL,
    createdate integer,
    createuser integer,
    popis text,
    typ character(1)
);


--
-- TOC entry 1369 (class 1259 OID 41252)
-- Dependencies: 1370 1
-- Name: role_id_seq; Type: SEQUENCE; Schema: web_app; Owner: -
--

CREATE SEQUENCE role_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1828 (class 0 OID 0)
-- Dependencies: 1369
-- Name: role_id_seq; Type: SEQUENCE OWNED BY; Schema: web_app; Owner: -
--

ALTER SEQUENCE role_id_seq OWNED BY role.id;


--
-- TOC entry 1371 (class 1259 OID 41262)
-- Dependencies: 1
-- Name: roleuzivatel; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE roleuzivatel (
    role integer NOT NULL,
    uzivatel integer NOT NULL
);


--
-- TOC entry 1373 (class 1259 OID 41268)
-- Dependencies: 1749 1
-- Name: uzivatel; Type: TABLE; Schema: web_app; Owner: -; Tablespace: 
--

CREATE TABLE uzivatel (
    id integer NOT NULL,
    login character varying(20) NOT NULL,
    jmeno character varying(20),
    prijmeni character varying(20),
    email character varying(100),
    poznamka text,
    createdate integer,
    createuser integer,
    heslo character(64),
    smazan integer DEFAULT 0 NOT NULL
);


--
-- TOC entry 1372 (class 1259 OID 41266)
-- Dependencies: 1373 1
-- Name: uzivatel_id_seq; Type: SEQUENCE; Schema: web_app; Owner: -
--

CREATE SEQUENCE uzivatel_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1832 (class 0 OID 0)
-- Dependencies: 1372
-- Name: uzivatel_id_seq; Type: SEQUENCE OWNED BY; Schema: web_app; Owner: -
--

ALTER SEQUENCE uzivatel_id_seq OWNED BY uzivatel.id;


SET search_path = dicomwl, pg_catalog;

--
-- TOC entry 1756 (class 2604 OID 49835)
-- Dependencies: 1393 1392 1393
-- Name: id_M; Type: DEFAULT; Schema: dicomwl; Owner: -
--

ALTER TABLE "ModalityName" ALTER COLUMN "id_M" SET DEFAULT nextval('"ModalityName_id_M_seq"'::regclass);


--
-- TOC entry 1757 (class 2604 OID 49842)
-- Dependencies: 1395 1394 1395
-- Name: id_VM; Type: DEFAULT; Schema: dicomwl; Owner: -
--

ALTER TABLE "VysMetodaName" ALTER COLUMN "id_VM" SET DEFAULT nextval('"VysMetodaName_id_VM_seq"'::regclass);


--
-- TOC entry 1758 (class 2604 OID 49849)
-- Dependencies: 1397 1396 1397
-- Name: idWL; Type: DEFAULT; Schema: dicomwl; Owner: -
--

ALTER TABLE "WorkList" ALTER COLUMN "idWL" SET DEFAULT nextval('"WorkList_idWL_seq"'::regclass);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1754 (class 2604 OID 49667)
-- Dependencies: 1379 1378 1379
-- Name: isrv_appg; Type: DEFAULT; Schema: srv_app; Owner: -
--

ALTER TABLE "Instance" ALTER COLUMN isrv_appg SET DEFAULT nextval('"InstanceTable_isrv_appg_seq"'::regclass);


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1742 (class 2604 OID 41201)
-- Dependencies: 1363 1364 1364
-- Name: id_AET; Type: DEFAULT; Schema: web_app; Owner: -
--

ALTER TABLE "ApplicationEntity" ALTER COLUMN "id_AET" SET DEFAULT nextval('"ApplicationEntity_id_AET_seq"'::regclass);


--
-- TOC entry 1747 (class 2604 OID 41256)
-- Dependencies: 1369 1370 1370
-- Name: id; Type: DEFAULT; Schema: web_app; Owner: -
--

ALTER TABLE role ALTER COLUMN id SET DEFAULT nextval('role_id_seq'::regclass);


--
-- TOC entry 1748 (class 2604 OID 41270)
-- Dependencies: 1373 1372 1373
-- Name: id; Type: DEFAULT; Schema: web_app; Owner: -
--

ALTER TABLE uzivatel ALTER COLUMN id SET DEFAULT nextval('uzivatel_id_seq'::regclass);


SET search_path = dicomwl, pg_catalog;

--
-- TOC entry 1802 (class 2606 OID 49860)
-- Dependencies: 1398 1398 1398
-- Name: Modalities_pkey; Type: CONSTRAINT; Schema: dicomwl; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Modalities"
    ADD CONSTRAINT "Modalities_pkey" PRIMARY KEY ("id_VysMetoda", "id_AET");


--
-- TOC entry 1800 (class 2606 OID 49856)
-- Dependencies: 1397 1397
-- Name: PK_ID_Worklist; Type: CONSTRAINT; Schema: dicomwl; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "WorkList"
    ADD CONSTRAINT "PK_ID_Worklist" PRIMARY KEY ("idWL");


--
-- TOC entry 1798 (class 2606 OID 49844)
-- Dependencies: 1395 1395
-- Name: PK_id; Type: CONSTRAINT; Schema: dicomwl; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "VysMetodaName"
    ADD CONSTRAINT "PK_id" PRIMARY KEY ("id_VM");


--
-- TOC entry 1796 (class 2606 OID 49837)
-- Dependencies: 1393 1393
-- Name: PK_id_ModalityName; Type: CONSTRAINT; Schema: dicomwl; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "ModalityName"
    ADD CONSTRAINT "PK_id_ModalityName" PRIMARY KEY ("id_M");


SET search_path = public, pg_catalog;

--
-- TOC entry 1766 (class 2606 OID 41096)
-- Dependencies: 1362 1362 1362 1362 1362
-- Name: PK_ImageCopy; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "ImageCopy"
    ADD CONSTRAINT "PK_ImageCopy" PRIMARY KEY ("Accession", "Source", "Destination", "SOPInstanceUID");


--
-- TOC entry 1764 (class 2606 OID 41089)
-- Dependencies: 1361 1361 1361
-- Name: PK_ImageForvard; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "ImageForward"
    ADD CONSTRAINT "PK_ImageForvard" PRIMARY KEY ("SourceApplication", "DestinationApplication");


--
-- TOC entry 1762 (class 2606 OID 40973)
-- Dependencies: 1360 1360
-- Name: fisaccess_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY fisaccess
    ADD CONSTRAINT fisaccess_pkey PRIMARY KEY (title);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1792 (class 2606 OID 49657)
-- Dependencies: 1377 1377
-- Name: pk_Image; Type: CONSTRAINT; Schema: srv_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Image"
    ADD CONSTRAINT "pk_Image" PRIMARY KEY ("SOPInsUID");


--
-- TOC entry 1794 (class 2606 OID 49670)
-- Dependencies: 1379 1379
-- Name: pk_InstanceTable; Type: CONSTRAINT; Schema: srv_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Instance"
    ADD CONSTRAINT "pk_InstanceTable" PRIMARY KEY (isrv_appg);


--
-- TOC entry 1786 (class 2606 OID 49632)
-- Dependencies: 1374 1374
-- Name: pk_Patient; Type: CONSTRAINT; Schema: srv_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Patient"
    ADD CONSTRAINT "pk_Patient" PRIMARY KEY ("PatID");


--
-- TOC entry 1790 (class 2606 OID 49647)
-- Dependencies: 1376 1376
-- Name: pk_Series; Type: CONSTRAINT; Schema: srv_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Series"
    ADD CONSTRAINT "pk_Series" PRIMARY KEY ("SerInsUID");


--
-- TOC entry 1788 (class 2606 OID 49637)
-- Dependencies: 1375 1375
-- Name: pk_Study; Type: CONSTRAINT; Schema: srv_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "Study"
    ADD CONSTRAINT "pk_Study" PRIMARY KEY ("StuInsUID");


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1776 (class 2606 OID 41248)
-- Dependencies: 1367 1367 1367
-- Name: PK_GRP_AET; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "GRP_AET"
    ADD CONSTRAINT "PK_GRP_AET" PRIMARY KEY ("id_AET", "id_GRP");


--
-- TOC entry 1772 (class 2606 OID 41229)
-- Dependencies: 1365 1365
-- Name: PK_StorageAccess; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "StorageAccess"
    ADD CONSTRAINT "PK_StorageAccess" PRIMARY KEY ("id_AET");


--
-- TOC entry 1778 (class 2606 OID 41261)
-- Dependencies: 1370 1370
-- Name: PK_id; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY role
    ADD CONSTRAINT "PK_id" PRIMARY KEY (id);


--
-- TOC entry 1768 (class 2606 OID 41204)
-- Dependencies: 1364 1364
-- Name: PK_id_AET; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "ApplicationEntity"
    ADD CONSTRAINT "PK_id_AET" PRIMARY KEY ("id_AET");


--
-- TOC entry 1774 (class 2606 OID 41244)
-- Dependencies: 1366 1366
-- Name: PK_id_GRP; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "GroupNames"
    ADD CONSTRAINT "PK_id_GRP" PRIMARY KEY ("id_GRP");


--
-- TOC entry 1780 (class 2606 OID 41265)
-- Dependencies: 1371 1371 1371
-- Name: PK_role; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY roleuzivatel
    ADD CONSTRAINT "PK_role" PRIMARY KEY (role, uzivatel);


--
-- TOC entry 1770 (class 2606 OID 41206)
-- Dependencies: 1364 1364
-- Name: U_Title; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "ApplicationEntity"
    ADD CONSTRAINT "U_Title" UNIQUE ("Title");


--
-- TOC entry 1782 (class 2606 OID 41278)
-- Dependencies: 1373 1373
-- Name: uzivatel_login_key; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY uzivatel
    ADD CONSTRAINT uzivatel_login_key UNIQUE (login);


--
-- TOC entry 1784 (class 2606 OID 41276)
-- Dependencies: 1373 1373
-- Name: uzivatel_pkey; Type: CONSTRAINT; Schema: web_app; Owner: -; Tablespace: 
--

ALTER TABLE ONLY uzivatel
    ADD CONSTRAINT uzivatel_pkey PRIMARY KEY (id);


SET search_path = srv_app, pg_catalog;

--
-- TOC entry 1806 (class 2606 OID 49671)
-- Dependencies: 1377 1379 1791
-- Name: fk_Image; Type: FK CONSTRAINT; Schema: srv_app; Owner: -
--

ALTER TABLE ONLY "Instance"
    ADD CONSTRAINT "fk_Image" FOREIGN KEY ("ImageUID") REFERENCES "Image"("SOPInsUID") ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 1803 (class 2606 OID 49638)
-- Dependencies: 1785 1374 1375
-- Name: fk_Patient; Type: FK CONSTRAINT; Schema: srv_app; Owner: -
--

ALTER TABLE ONLY "Study"
    ADD CONSTRAINT "fk_Patient" FOREIGN KEY ("PatParent") REFERENCES "Patient"("PatID") ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 1805 (class 2606 OID 49658)
-- Dependencies: 1376 1377 1789
-- Name: fk_Series; Type: FK CONSTRAINT; Schema: srv_app; Owner: -
--

ALTER TABLE ONLY "Image"
    ADD CONSTRAINT "fk_Series" FOREIGN KEY ("SerParent") REFERENCES "Series"("SerInsUID") ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 1804 (class 2606 OID 49648)
-- Dependencies: 1787 1375 1376
-- Name: fk_Study; Type: FK CONSTRAINT; Schema: srv_app; Owner: -
--

ALTER TABLE ONLY "Series"
    ADD CONSTRAINT "fk_Study" FOREIGN KEY ("StuParent") REFERENCES "Study"("StuInsUID") ON UPDATE CASCADE ON DELETE CASCADE;


--
-- TOC entry 1812 (class 0 OID 0)
-- Dependencies: 6
-- Name: public; Type: ACL; Schema: -; Owner: -
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


SET search_path = public, pg_catalog;

--
-- TOC entry 1813 (class 0 OID 0)
-- Dependencies: 30
-- Name: dblink_connect_u(text); Type: ACL; Schema: public; Owner: -
--

REVOKE ALL ON FUNCTION dblink_connect_u(text) FROM PUBLIC;
REVOKE ALL ON FUNCTION dblink_connect_u(text) FROM postgres;
GRANT ALL ON FUNCTION dblink_connect_u(text) TO postgres;


--
-- TOC entry 1814 (class 0 OID 0)
-- Dependencies: 31
-- Name: dblink_connect_u(text, text); Type: ACL; Schema: public; Owner: -
--

REVOKE ALL ON FUNCTION dblink_connect_u(text, text) FROM PUBLIC;
REVOKE ALL ON FUNCTION dblink_connect_u(text, text) FROM postgres;
GRANT ALL ON FUNCTION dblink_connect_u(text, text) TO postgres;


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1815 (class 0 OID 0)
-- Dependencies: 1364
-- Name: ApplicationEntity; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE "ApplicationEntity" FROM PUBLIC;
REVOKE ALL ON TABLE "ApplicationEntity" FROM postgres;
GRANT ALL ON TABLE "ApplicationEntity" TO postgres;
GRANT ALL ON TABLE "ApplicationEntity" TO web_app;


SET search_path = public, pg_catalog;

--
-- TOC entry 1819 (class 0 OID 0)
-- Dependencies: 1360
-- Name: fisaccess; Type: ACL; Schema: public; Owner: -
--

REVOKE ALL ON TABLE fisaccess FROM PUBLIC;
REVOKE ALL ON TABLE fisaccess FROM postgres;
GRANT ALL ON TABLE fisaccess TO postgres;
GRANT ALL ON TABLE fisaccess TO PUBLIC;


SET search_path = web_app, pg_catalog;

--
-- TOC entry 1820 (class 0 OID 0)
-- Dependencies: 1367
-- Name: GRP_AET; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE "GRP_AET" FROM PUBLIC;
REVOKE ALL ON TABLE "GRP_AET" FROM postgres;
GRANT ALL ON TABLE "GRP_AET" TO postgres;
GRANT ALL ON TABLE "GRP_AET" TO web_app;


--
-- TOC entry 1821 (class 0 OID 0)
-- Dependencies: 1368
-- Name: GroupNames_id_GRP_seq; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON SEQUENCE "GroupNames_id_GRP_seq" FROM PUBLIC;
REVOKE ALL ON SEQUENCE "GroupNames_id_GRP_seq" FROM postgres;
GRANT ALL ON SEQUENCE "GroupNames_id_GRP_seq" TO postgres;
GRANT ALL ON SEQUENCE "GroupNames_id_GRP_seq" TO web_app;


--
-- TOC entry 1822 (class 0 OID 0)
-- Dependencies: 1366
-- Name: GroupNames; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE "GroupNames" FROM PUBLIC;
REVOKE ALL ON TABLE "GroupNames" FROM postgres;
GRANT ALL ON TABLE "GroupNames" TO postgres;
GRANT ALL ON TABLE "GroupNames" TO web_app;


--
-- TOC entry 1824 (class 0 OID 0)
-- Dependencies: 1365
-- Name: StorageAccess; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE "StorageAccess" FROM PUBLIC;
REVOKE ALL ON TABLE "StorageAccess" FROM postgres;
GRANT ALL ON TABLE "StorageAccess" TO postgres;
GRANT ALL ON TABLE "StorageAccess" TO web_app;


--
-- TOC entry 1826 (class 0 OID 0)
-- Dependencies: 1363
-- Name: ApplicationEntity_id_AET_seq; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON SEQUENCE "ApplicationEntity_id_AET_seq" FROM PUBLIC;
REVOKE ALL ON SEQUENCE "ApplicationEntity_id_AET_seq" FROM postgres;
GRANT ALL ON SEQUENCE "ApplicationEntity_id_AET_seq" TO postgres;
GRANT ALL ON SEQUENCE "ApplicationEntity_id_AET_seq" TO web_app;


--
-- TOC entry 1827 (class 0 OID 0)
-- Dependencies: 1370
-- Name: role; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE role FROM PUBLIC;
REVOKE ALL ON TABLE role FROM postgres;
GRANT ALL ON TABLE role TO postgres;
GRANT ALL ON TABLE role TO web_app;


--
-- TOC entry 1829 (class 0 OID 0)
-- Dependencies: 1369
-- Name: role_id_seq; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON SEQUENCE role_id_seq FROM PUBLIC;
REVOKE ALL ON SEQUENCE role_id_seq FROM postgres;
GRANT ALL ON SEQUENCE role_id_seq TO postgres;
GRANT ALL ON SEQUENCE role_id_seq TO web_app;


--
-- TOC entry 1830 (class 0 OID 0)
-- Dependencies: 1371
-- Name: roleuzivatel; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE roleuzivatel FROM PUBLIC;
REVOKE ALL ON TABLE roleuzivatel FROM postgres;
GRANT ALL ON TABLE roleuzivatel TO postgres;
GRANT ALL ON TABLE roleuzivatel TO web_app;


--
-- TOC entry 1831 (class 0 OID 0)
-- Dependencies: 1373
-- Name: uzivatel; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON TABLE uzivatel FROM PUBLIC;
REVOKE ALL ON TABLE uzivatel FROM postgres;
GRANT ALL ON TABLE uzivatel TO postgres;
GRANT ALL ON TABLE uzivatel TO web_app;


--
-- TOC entry 1833 (class 0 OID 0)
-- Dependencies: 1372
-- Name: uzivatel_id_seq; Type: ACL; Schema: web_app; Owner: -
--

REVOKE ALL ON SEQUENCE uzivatel_id_seq FROM PUBLIC;
REVOKE ALL ON SEQUENCE uzivatel_id_seq FROM postgres;
GRANT ALL ON SEQUENCE uzivatel_id_seq TO postgres;
GRANT ALL ON SEQUENCE uzivatel_id_seq TO web_app;


-- Completed on 2009-10-30 20:49:24 CET

--
-- PostgreSQL database dump complete
--

