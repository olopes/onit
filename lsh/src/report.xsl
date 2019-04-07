<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="html" encoding="utf-8" doctype-system="about:legacy-compat" /> 

<xsl:template match="/">
<html>
<head>
    <title>Test run <xsl:value-of select="files/@created" /></title>
    <style>
    body {
        color: #000000;
        background-color: #FFFFFF;
    }

    /* All views: standard link format*/
    a:link {
        color: #284FA8;
        text-decoration: underline;
    }

    /* All views: standard link - visited format */
    a:visited {
        color: #00CB40;
        text-decoration: underline;
    }

    /* All views: standard link - activated format */
    a:active {
        color: #FF0040;
        text-decoration: underline;
    }
    
    table {
        margin-left:auto; 
        margin-right:auto;
    }

    th {
        text-align: center;
        color: #FFFFFF;
        background-color: #6688D4;
        font-family: sans-serif;
        font-size: 120%;
        font-weight: bold;
        white-space: nowrap;
        padding-left: 4px;
        padding-right: 4px;
    }
    
    td {
        text-align: left;
        padding-left: 10px;
        padding-right: 20px;
        background-color: #DAE7FE;
        font-family: monospace;
    }

    td.teststatus {
        font-weight: bold;
        font-family: sans-serif;
    }

    td.testtime {
        text-align: right;
        font-weight: bold;
        font-family: sans-serif;
    }
    .error td.teststatus, .error td.testtime {
        background-color: #FF0000;
    }

    .failure td.teststatus, .failure td.testtime {
        background-color: #ff352f;
    }

    .skipped td.teststatus, .skipped td.testtime {
        background-color: #FFEA20;
    }

    .pass td.teststatus, .pass td.testtime {
        background-color: #A7FC9D;
    }

    hr {
        background-color: #6688D4;
        height: 2px;
        border: none;
    }

    /* #1bea59 */
    label {
        cursor: pointer;
    }
    
    label.pass {
        background: #1bea59;
    }
    
    label.failure {
        background: #ff352f;
    }
    
    .testsuite input[type=checkbox] {
        /*opacity:0;*/
        /*position: absolute;*/
        /*top: -9999px;*/
        /*left: -9999px;*/
        /*display:none;*/
        display: none;
    }

    /* Default State */
    .testsuite div.testset {
        display:none;
    }

    /* Toggled State */
    .testsuite input[type=checkbox]:checked ~ div.testset {
        display:block;
    }
</style>
</head>
<body>
<xsl:apply-templates />
</body>
</html>
</xsl:template>

<xsl:template match="files">
    <h1>Test run
    <xsl:value-of select="@created" />
    </h1>
    <hr />
    <xsl:for-each select="file">
        <xsl:apply-templates select="document(.)/testsuites" />
    </xsl:for-each>
</xsl:template>

<xsl:template match="testsuites">
    <xsl:apply-templates select="testsuite" />
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
    <label for="{@name}">
        <xsl:attribute name="class">
            <xsl:choose>
                <xsl:when test="@failures > 0 or @errors > 0">failure</xsl:when>
                <xsl:otherwise>pass</xsl:otherwise>
            </xsl:choose>
        </xsl:attribute>
        <xsl:value-of select="@name" />
    </label>
    <div class="testset">
        <table width="80%" cellspacing="1" cellpadding="1" border="0">
            <tr>
                <th nowrap="nowrap" width="80%">Test name</th>
                <th nowrap="nowrap">Status</th>
                <th nowrap="nowrap" align="right">Time(s)</th>
            </tr>
    
    
            <xsl:apply-templates select="testcase" />
    
        </table>
        <a href="{$srcurl}">Source coverage</a>
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

    <xsl:if test="failure">
        <tr class="faildetail"><td colspan="3" nowrap="nowrap"><pre><xsl:value-of select="failure"/></pre></td></tr>
    </xsl:if>
    <xsl:if test="error">
        <tr class="faildetail"><td colspan="3" nowrap="nowrap"><pre><xsl:value-of select="error"/></pre></td></tr>
    </xsl:if>
</xsl:template>

</xsl:stylesheet>
