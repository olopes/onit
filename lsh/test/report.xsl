<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:str="http://exslt.org/strings"
    xmlns:date="http://exslt.org/dates-and-times"
    extension-element-prefixes="date str">

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
    td.faildetail div a {color: #00CB40;text-decoration: underline;}
    td.teststatus {font-weight: bold;text-align:center;}
    td.testtime {text-align: right;}
    .error td.teststatus {background-color: #FF0000; color: #eee;}
    .failure td.teststatus {background-color: #ff352f; color: #eee;}
    .skipped td.teststatus {background-color: #FFEA20;}
    .pass td.teststatus {background-color: #A7FC9D;}
    hr {background-color: #6688D4;height: 2px;border: none;}
    label {cursor: pointer;}
    label span {margin-right: 5px;}
    label span.testfile {width: 40ex;display: inline-block;text-decoration: underline;}
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
    .timestamp {font-size:small;text-align:right;font-style:italic;}
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
    
    <div class="timestamp">Build run at <span id="thetime"><xsl:value-of select="concat(date:date(@created), ' ', date:time(@created))" /></span><br />
    <span id="thetime"><xsl:value-of select="@sysinfo" /></span></div>
</xsl:template>

<xsl:template match="testsuite">
<xsl:variable name="src_file">
    <xsl:choose>
    <xsl:when test="starts-with(@name, 'build/')">
        <xsl:value-of select="concat(substring-before(substring(@name, 7), '.t'), '.c')" />
    </xsl:when>
    <xsl:otherwise>
        <xsl:value-of select="@name" />
    </xsl:otherwise>
    </xsl:choose>
</xsl:variable>
<xsl:variable name="srcurl">
    <xsl:value-of select="concat('../coverage/', $src_file, '.gcov.html')"/>
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
        <span class="testfile"><xsl:value-of select="$src_file" /></span>
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
            <xsl:when test="skipped">skipped</xsl:when>
            <xsl:otherwise>pass</xsl:otherwise>
        </xsl:choose>
        </xsl:attribute>
        
        <td class="testname"><xsl:value-of select="@name"/></td>
        
        <td class="teststatus">
            <xsl:choose>
                <xsl:when test="failure">Failure</xsl:when>
                <xsl:when test="error">Error</xsl:when>
                <xsl:when test="skipped">Skipped</xsl:when>
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
    <xsl:for-each select="str:tokenize($error, '&#10;')">
        <xsl:call-template name="formaterrorline">
            <xsl:with-param name="line" select="." />
        </xsl:call-template>
    </xsl:for-each>
  
    </div>
</xsl:template>

<xsl:template name="formaterrorline">
    <xsl:param name = "line" />
    <xsl:variable name="file" select="substring-before($line, ':')" />
    <xsl:variable name="linenum" select="substring-before(substring-after($line, ':'), ':')" />

    <xsl:choose>
        <xsl:when test="$file and $linenum">
            <a>
                <xsl:attribute name="href">
                    ../coverage/<xsl:value-of select="$file" />.gcov.html#<xsl:value-of select="$linenum" />
                </xsl:attribute>
                <xsl:value-of select="$file" />:<xsl:value-of select="$linenum" />
            </a>:<xsl:value-of select="substring-after(substring-after($line, ':'), ':')" />
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$line" />
        </xsl:otherwise>
    </xsl:choose>
    <br />
</xsl:template>

</xsl:stylesheet>
