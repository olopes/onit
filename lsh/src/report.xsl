<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:str="http://exslt.org/strings"
    xmlns:regex="http://exslt.org/regular-expressions"
    xmlns:date="http://exslt.org/dates-and-times"
    extension-element-prefixes="date regex str xs">

<xsl:output method="html" encoding="utf-8" doctype-system="about:legacy-compat" /> 

<xsl:template match="/">
<html>
<head>
    <title>Test Execution Result</title>
    <style>
    /* styles inspired from lcov and genhtml */
    body { color: #000000; background-color: #FFFFFF; font-family: sans-serif;}
    a:link { color: #284FA8; text-decoration: underline; }
    a:visited {color: #00CB40;text-decoration: underline;}
    a:active {color: #FF0040;text-decoration: underline;}
    table {margin-left:auto; margin-right:auto;margin-top:10px; margin-bottom:20px;}
    th {text-align: center;color: #FFFFFF;background-color: #6688D4;font-family: sans-serif;font-size: 120%;font-weight: bold;white-space: nowrap;padding-left: 4px;padding-right: 4px;}
    td {text-align: left;padding-left: 10px;padding-right: 20px;background-color: #DAE7FE;}
    td.faildetail {font-family: monospace;margin-left: 2em;background-color: #36454f;color:#30FF00;}
    td.faildetail div {margin: 1em;white-space: pre;}
    td.teststatus {font-weight: bold;text-align:center;}
    td.testtime {text-align: right;}
    .error td.teststatus {background-color: #FF0000; color: #eee;}
    .failure td.teststatus {background-color: #ff352f; color: #eee;}
    .skipped td.teststatus {background-color: #FFEA20;}
    .pass td.teststatus {background-color: #A7FC9D;}
    hr {background-color: #6688D4;height: 2px;border: none;}
    label {cursor: pointer;}
    label span {margin-right: 5px;}
    label span.testfile {width: 60ex;display: inline-block;text-decoration: underline;}
    label span.count {width: 10ex;display: inline-block;background: #dddddd;text-align:right}
    div.duration {font-size:smaller;border-bottom: 1px dotted #ccc;color: #555;}
    span.square {border: 1px solid #000000;height: 1ex;width: 1ex;display:inline-block;color: white;}
    .pass span.square {background: #1bea59;}
    .error span.square {background: #ff352f;}
    .failure span.square {background: #5477a7;}
    .testsuite .coverage {float:right;}
    .testsuite input[type=checkbox] {display: none;}
    .testsuite div.testset {display:none;}
    .testsuite input[type=checkbox]:checked ~ div.testset {display:block;}
</style>
</head>
<body>
    <xsl:apply-templates />
</body>
</html>
</xsl:template>

<xsl:template match="files">
    <h1>Test Execution Result</h1>
    <hr />
    
    <xsl:for-each select="file">
        <xsl:apply-templates select="document(.)/testsuites/testsuite" />
    </xsl:for-each>
    
    <div>Report generated at <span id="thetime"><xsl:value-of select="@created" /></span></div>
    <script type="text/javascript">
    document.getElementById('thetime').innerText = new Date('<xsl:value-of select="@created" />').toString();
    </script>
</xsl:template>

<xsl:template match="testsuite">
<xsl:variable name="srcurl">
    <xsl:value-of select="concat('../coverage/', @name, '.gcov.frameset.html')"/>
</xsl:variable>
<div class="testsuite">
    <input id="{@name}" type="checkbox">
        <xsl:if test="@failures > 0 or @errors > 0">
            <xsl:attribute name="checked">checked</xsl:attribute>
        </xsl:if>
    </input>
    <a class="coverage" href="{$srcurl}">Coverage</a>
    <label for="{@name}">
        <xsl:attribute name="class">
            <xsl:choose>
                <xsl:when test="@failures > 0 or @errors > 0">error</xsl:when>
                <xsl:otherwise>pass</xsl:otherwise>
            </xsl:choose>
        </xsl:attribute>
        <span class="square"></span>
        <span class="testfile"><xsl:value-of select="@name" /></span>
        <span>Runs: <span class="count"><xsl:value-of select="@tests - @skipped" /> / <xsl:value-of select="@tests" /></span></span>
        <span class="error"><span class="square"></span>Errors: <span class="count"><xsl:value-of select="@errors" /></span></span>
        <span class="failure"><span class="square"></span>Failures: <span class="count"><xsl:value-of select="@failures" /></span></span>
    </label>
    <div class="duration">Finished after <xsl:value-of select="format-number(@time, '#0.000')"/> seconds</div>
    <div class="testset">
        <table width="80%" cellspacing="1" cellpadding="1" border="0">
            <tr>
                <th nowrap="nowrap" width="80%">Test name</th>
                <th nowrap="nowrap">Status</th>
                <th nowrap="nowrap" align="right">Time(s)</th>
            </tr>
    
    
            <xsl:apply-templates select="testcase" />
    
        </table>
    </div>

</div>
<hr />
</xsl:template>


<xsl:template match="testcase">
    <tr valign="top">
        <xsl:attribute name="class">
        <xsl:choose>
            <xsl:when test="error">error</xsl:when>
            <xsl:when test="failure">failure</xsl:when>
            <xsl:otherwise>pass</xsl:otherwise>
        </xsl:choose>
        </xsl:attribute>
        
        <td class="testname"><xsl:value-of select="@name"/></td>
        
        <td class="teststatus">
            <xsl:choose>
                <xsl:when test="failure">Failure</xsl:when>
                <xsl:when test="error">Error</xsl:when>
                <xsl:otherwise>Success</xsl:otherwise>
            </xsl:choose>
        </td>
        
        <td class="testtime"><xsl:value-of select="format-number(@time, '#0.000')"/></td>
    </tr>

    <xsl:if test="failure or error">
        <tr>
            <td colspan="3" class="faildetail">
                <xsl:call-template name="errormsg">
                    <xsl:with-param name="error" select="error | failure" />
                </xsl:call-template>
            </td>
        </tr>
    </xsl:if>
</xsl:template>

<xsl:template name="errormsg">
    <xsl:param name = "error" />
    <div>
    <!--<div><xsl:value-of select="regex:replace($error,'((.+)\:(\d+))\:','g','&lt;a href=&quot;../coverage/$2.gcov.html#$3&quot;&gt;$1&lt;/a&gt;:')" disable-output-escaping="yes" />-->
    
    
    <xsl:for-each select="str:tokenize($error, '&#10;')">
        <xsl:value-of select="." /><br />
    </xsl:for-each>
  
    </div>
</xsl:template>

</xsl:stylesheet>
