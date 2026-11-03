<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings><setting alwaysvectorfont="no"/><setting verticaltext="up"/></settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="yes" active="yes"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="yes" active="yes"/>
<layer number="17" name="Pads" color="2" fill="1" visible="yes" active="yes"/>
<layer number="18" name="Vias" color="2" fill="1" visible="yes" active="yes"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="yes" active="yes"/>
<layer number="20" name="Dimension" color="24" fill="1" visible="yes" active="yes"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="yes" active="yes"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="yes" active="yes"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="yes" active="yes"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="yes" active="yes"/>
<layer number="25" name="tNames" color="7" fill="1" visible="yes" active="yes"/>
<layer number="26" name="bNames" color="7" fill="1" visible="yes" active="yes"/>
<layer number="27" name="tValues" color="7" fill="1" visible="yes" active="yes"/>
<layer number="28" name="bValues" color="7" fill="1" visible="yes" active="yes"/>
<layer number="29" name="tStop" color="7" fill="3" visible="yes" active="yes"/>
<layer number="30" name="bStop" color="7" fill="6" visible="yes" active="yes"/>
<layer number="31" name="tCream" color="7" fill="4" visible="yes" active="yes"/>
<layer number="32" name="bCream" color="7" fill="5" visible="yes" active="yes"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="yes" active="yes"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="yes" active="yes"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="yes" active="yes"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="yes" active="yes"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="yes" active="yes"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="yes" active="yes"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="yes" active="yes"/>
<layer number="44" name="Drills" color="7" fill="1" visible="yes" active="yes"/>
<layer number="45" name="Holes" color="7" fill="1" visible="yes" active="yes"/>
<layer number="46" name="Milling" color="3" fill="1" visible="yes" active="yes"/>
<layer number="47" name="Measures" color="7" fill="1" visible="yes" active="yes"/>
<layer number="48" name="Document" color="7" fill="1" visible="yes" active="yes"/>
<layer number="49" name="Reference" color="7" fill="1" visible="yes" active="yes"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="yes" active="yes"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="yes" active="yes"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="yes" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic>
<libraries>
<library name="cloudline">
<packages>
<package name="CP_Radial_D8.0mm_P3.50mm">
<description>Capacitor_THT:CP_Radial_D8.0mm_P3.50mm (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-2.6597" y1="2.315" x2="-1.8597" y2="2.315" width="0.12" layer="21"/>
<wire x1="-2.2597" y1="2.715" x2="-2.2597" y2="1.915" width="0.12" layer="21"/>
<wire x1="1.75" y1="4.08" x2="1.75" y2="-4.08" width="0.12" layer="21"/>
<wire x1="1.79" y1="4.08" x2="1.79" y2="-4.08" width="0.12" layer="21"/>
<wire x1="1.83" y1="4.08" x2="1.83" y2="-4.08" width="0.12" layer="21"/>
<wire x1="1.87" y1="4.079" x2="1.87" y2="-4.079" width="0.12" layer="21"/>
<wire x1="1.91" y1="4.077" x2="1.91" y2="-4.077" width="0.12" layer="21"/>
<wire x1="1.95" y1="4.076" x2="1.95" y2="-4.076" width="0.12" layer="21"/>
<wire x1="1.99" y1="4.074" x2="1.99" y2="-4.074" width="0.12" layer="21"/>
<wire x1="2.03" y1="4.071" x2="2.03" y2="-4.071" width="0.12" layer="21"/>
<wire x1="2.07" y1="4.068" x2="2.07" y2="-4.068" width="0.12" layer="21"/>
<wire x1="2.11" y1="4.065" x2="2.11" y2="-4.065" width="0.12" layer="21"/>
<wire x1="2.15" y1="4.061" x2="2.15" y2="-4.061" width="0.12" layer="21"/>
<wire x1="2.19" y1="4.057" x2="2.19" y2="-4.057" width="0.12" layer="21"/>
<wire x1="2.23" y1="4.052" x2="2.23" y2="-4.052" width="0.12" layer="21"/>
<wire x1="2.27" y1="4.048" x2="2.27" y2="-4.048" width="0.12" layer="21"/>
<wire x1="2.31" y1="4.042" x2="2.31" y2="-4.042" width="0.12" layer="21"/>
<wire x1="2.35" y1="4.037" x2="2.35" y2="-4.037" width="0.12" layer="21"/>
<wire x1="2.39" y1="4.03" x2="2.39" y2="-4.03" width="0.12" layer="21"/>
<wire x1="2.43" y1="4.024" x2="2.43" y2="-4.024" width="0.12" layer="21"/>
<wire x1="2.471" y1="4.017" x2="2.471" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.471" y1="-1.04" x2="2.471" y2="-4.017" width="0.12" layer="21"/>
<wire x1="2.511" y1="4.01" x2="2.511" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.511" y1="-1.04" x2="2.511" y2="-4.01" width="0.12" layer="21"/>
<wire x1="2.551" y1="4.002" x2="2.551" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.551" y1="-1.04" x2="2.551" y2="-4.002" width="0.12" layer="21"/>
<wire x1="2.591" y1="3.994" x2="2.591" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.591" y1="-1.04" x2="2.591" y2="-3.994" width="0.12" layer="21"/>
<wire x1="2.631" y1="3.985" x2="2.631" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.631" y1="-1.04" x2="2.631" y2="-3.985" width="0.12" layer="21"/>
<wire x1="2.671" y1="3.976" x2="2.671" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.671" y1="-1.04" x2="2.671" y2="-3.976" width="0.12" layer="21"/>
<wire x1="2.711" y1="3.967" x2="2.711" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.711" y1="-1.04" x2="2.711" y2="-3.967" width="0.12" layer="21"/>
<wire x1="2.751" y1="3.957" x2="2.751" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.751" y1="-1.04" x2="2.751" y2="-3.957" width="0.12" layer="21"/>
<wire x1="2.791" y1="3.947" x2="2.791" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.791" y1="-1.04" x2="2.791" y2="-3.947" width="0.12" layer="21"/>
<wire x1="2.831" y1="3.936" x2="2.831" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.831" y1="-1.04" x2="2.831" y2="-3.936" width="0.12" layer="21"/>
<wire x1="2.871" y1="3.925" x2="2.871" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.871" y1="-1.04" x2="2.871" y2="-3.925" width="0.12" layer="21"/>
<wire x1="2.911" y1="3.914" x2="2.911" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.911" y1="-1.04" x2="2.911" y2="-3.914" width="0.12" layer="21"/>
<wire x1="2.951" y1="3.902" x2="2.951" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.951" y1="-1.04" x2="2.951" y2="-3.902" width="0.12" layer="21"/>
<wire x1="2.991" y1="3.889" x2="2.991" y2="1.04" width="0.12" layer="21"/>
<wire x1="2.991" y1="-1.04" x2="2.991" y2="-3.889" width="0.12" layer="21"/>
<wire x1="3.031" y1="3.877" x2="3.031" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.031" y1="-1.04" x2="3.031" y2="-3.877" width="0.12" layer="21"/>
<wire x1="3.071" y1="3.863" x2="3.071" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.071" y1="-1.04" x2="3.071" y2="-3.863" width="0.12" layer="21"/>
<wire x1="3.111" y1="3.85" x2="3.111" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.111" y1="-1.04" x2="3.111" y2="-3.85" width="0.12" layer="21"/>
<wire x1="3.151" y1="3.835" x2="3.151" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.151" y1="-1.04" x2="3.151" y2="-3.835" width="0.12" layer="21"/>
<wire x1="3.191" y1="3.821" x2="3.191" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.191" y1="-1.04" x2="3.191" y2="-3.821" width="0.12" layer="21"/>
<wire x1="3.231" y1="3.805" x2="3.231" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.231" y1="-1.04" x2="3.231" y2="-3.805" width="0.12" layer="21"/>
<wire x1="3.271" y1="3.79" x2="3.271" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.271" y1="-1.04" x2="3.271" y2="-3.79" width="0.12" layer="21"/>
<wire x1="3.311" y1="3.774" x2="3.311" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.311" y1="-1.04" x2="3.311" y2="-3.774" width="0.12" layer="21"/>
<wire x1="3.351" y1="3.757" x2="3.351" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.351" y1="-1.04" x2="3.351" y2="-3.757" width="0.12" layer="21"/>
<wire x1="3.391" y1="3.74" x2="3.391" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.391" y1="-1.04" x2="3.391" y2="-3.74" width="0.12" layer="21"/>
<wire x1="3.431" y1="3.722" x2="3.431" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.431" y1="-1.04" x2="3.431" y2="-3.722" width="0.12" layer="21"/>
<wire x1="3.471" y1="3.704" x2="3.471" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.471" y1="-1.04" x2="3.471" y2="-3.704" width="0.12" layer="21"/>
<wire x1="3.511" y1="3.686" x2="3.511" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.511" y1="-1.04" x2="3.511" y2="-3.686" width="0.12" layer="21"/>
<wire x1="3.551" y1="3.666" x2="3.551" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.551" y1="-1.04" x2="3.551" y2="-3.666" width="0.12" layer="21"/>
<wire x1="3.591" y1="3.647" x2="3.591" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.591" y1="-1.04" x2="3.591" y2="-3.647" width="0.12" layer="21"/>
<wire x1="3.631" y1="3.627" x2="3.631" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.631" y1="-1.04" x2="3.631" y2="-3.627" width="0.12" layer="21"/>
<wire x1="3.671" y1="3.606" x2="3.671" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.671" y1="-1.04" x2="3.671" y2="-3.606" width="0.12" layer="21"/>
<wire x1="3.711" y1="3.584" x2="3.711" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.711" y1="-1.04" x2="3.711" y2="-3.584" width="0.12" layer="21"/>
<wire x1="3.751" y1="3.562" x2="3.751" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.751" y1="-1.04" x2="3.751" y2="-3.562" width="0.12" layer="21"/>
<wire x1="3.791" y1="3.54" x2="3.791" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.791" y1="-1.04" x2="3.791" y2="-3.54" width="0.12" layer="21"/>
<wire x1="3.831" y1="3.517" x2="3.831" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.831" y1="-1.04" x2="3.831" y2="-3.517" width="0.12" layer="21"/>
<wire x1="3.871" y1="3.493" x2="3.871" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.871" y1="-1.04" x2="3.871" y2="-3.493" width="0.12" layer="21"/>
<wire x1="3.911" y1="3.469" x2="3.911" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.911" y1="-1.04" x2="3.911" y2="-3.469" width="0.12" layer="21"/>
<wire x1="3.951" y1="3.444" x2="3.951" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.951" y1="-1.04" x2="3.951" y2="-3.444" width="0.12" layer="21"/>
<wire x1="3.991" y1="3.418" x2="3.991" y2="1.04" width="0.12" layer="21"/>
<wire x1="3.991" y1="-1.04" x2="3.991" y2="-3.418" width="0.12" layer="21"/>
<wire x1="4.031" y1="3.392" x2="4.031" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.031" y1="-1.04" x2="4.031" y2="-3.392" width="0.12" layer="21"/>
<wire x1="4.071" y1="3.365" x2="4.071" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.071" y1="-1.04" x2="4.071" y2="-3.365" width="0.12" layer="21"/>
<wire x1="4.111" y1="3.338" x2="4.111" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.111" y1="-1.04" x2="4.111" y2="-3.338" width="0.12" layer="21"/>
<wire x1="4.151" y1="3.309" x2="4.151" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.151" y1="-1.04" x2="4.151" y2="-3.309" width="0.12" layer="21"/>
<wire x1="4.191" y1="3.28" x2="4.191" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.191" y1="-1.04" x2="4.191" y2="-3.28" width="0.12" layer="21"/>
<wire x1="4.231" y1="3.25" x2="4.231" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.231" y1="-1.04" x2="4.231" y2="-3.25" width="0.12" layer="21"/>
<wire x1="4.271" y1="3.22" x2="4.271" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.271" y1="-1.04" x2="4.271" y2="-3.22" width="0.12" layer="21"/>
<wire x1="4.311" y1="3.189" x2="4.311" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.311" y1="-1.04" x2="4.311" y2="-3.189" width="0.12" layer="21"/>
<wire x1="4.351" y1="3.156" x2="4.351" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.351" y1="-1.04" x2="4.351" y2="-3.156" width="0.12" layer="21"/>
<wire x1="4.391" y1="3.124" x2="4.391" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.391" y1="-1.04" x2="4.391" y2="-3.124" width="0.12" layer="21"/>
<wire x1="4.431" y1="3.09" x2="4.431" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.431" y1="-1.04" x2="4.431" y2="-3.09" width="0.12" layer="21"/>
<wire x1="4.471" y1="3.055" x2="4.471" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.471" y1="-1.04" x2="4.471" y2="-3.055" width="0.12" layer="21"/>
<wire x1="4.511" y1="3.019" x2="4.511" y2="1.04" width="0.12" layer="21"/>
<wire x1="4.511" y1="-1.04" x2="4.511" y2="-3.019" width="0.12" layer="21"/>
<wire x1="4.551" y1="2.983" x2="4.551" y2="-2.983" width="0.12" layer="21"/>
<wire x1="4.591" y1="2.945" x2="4.591" y2="-2.945" width="0.12" layer="21"/>
<wire x1="4.631" y1="2.907" x2="4.631" y2="-2.907" width="0.12" layer="21"/>
<wire x1="4.671" y1="2.867" x2="4.671" y2="-2.867" width="0.12" layer="21"/>
<wire x1="4.711" y1="2.826" x2="4.711" y2="-2.826" width="0.12" layer="21"/>
<wire x1="4.751" y1="2.784" x2="4.751" y2="-2.784" width="0.12" layer="21"/>
<wire x1="4.791" y1="2.741" x2="4.791" y2="-2.741" width="0.12" layer="21"/>
<wire x1="4.831" y1="2.697" x2="4.831" y2="-2.697" width="0.12" layer="21"/>
<wire x1="4.871" y1="2.651" x2="4.871" y2="-2.651" width="0.12" layer="21"/>
<wire x1="4.911" y1="2.604" x2="4.911" y2="-2.604" width="0.12" layer="21"/>
<wire x1="4.951" y1="2.556" x2="4.951" y2="-2.556" width="0.12" layer="21"/>
<wire x1="4.991" y1="2.505" x2="4.991" y2="-2.505" width="0.12" layer="21"/>
<wire x1="5.031" y1="2.454" x2="5.031" y2="-2.454" width="0.12" layer="21"/>
<wire x1="5.071" y1="2.4" x2="5.071" y2="-2.4" width="0.12" layer="21"/>
<wire x1="5.111" y1="2.345" x2="5.111" y2="-2.345" width="0.12" layer="21"/>
<wire x1="5.151" y1="2.287" x2="5.151" y2="-2.287" width="0.12" layer="21"/>
<wire x1="5.191" y1="2.228" x2="5.191" y2="-2.228" width="0.12" layer="21"/>
<wire x1="5.231" y1="2.166" x2="5.231" y2="-2.166" width="0.12" layer="21"/>
<wire x1="5.271" y1="2.102" x2="5.271" y2="-2.102" width="0.12" layer="21"/>
<wire x1="5.311" y1="2.034" x2="5.311" y2="-2.034" width="0.12" layer="21"/>
<wire x1="5.351" y1="1.964" x2="5.351" y2="-1.964" width="0.12" layer="21"/>
<wire x1="5.391" y1="1.89" x2="5.391" y2="-1.89" width="0.12" layer="21"/>
<wire x1="5.431" y1="1.813" x2="5.431" y2="-1.813" width="0.12" layer="21"/>
<wire x1="5.471" y1="1.731" x2="5.471" y2="-1.731" width="0.12" layer="21"/>
<wire x1="5.511" y1="1.645" x2="5.511" y2="-1.645" width="0.12" layer="21"/>
<wire x1="5.551" y1="1.552" x2="5.551" y2="-1.552" width="0.12" layer="21"/>
<wire x1="5.591" y1="1.453" x2="5.591" y2="-1.453" width="0.12" layer="21"/>
<wire x1="5.631" y1="1.346" x2="5.631" y2="-1.346" width="0.12" layer="21"/>
<wire x1="5.671" y1="1.229" x2="5.671" y2="-1.229" width="0.12" layer="21"/>
<wire x1="5.711" y1="1.098" x2="5.711" y2="-1.098" width="0.12" layer="21"/>
<wire x1="5.751" y1="0.948" x2="5.751" y2="-0.948" width="0.12" layer="21"/>
<wire x1="5.791" y1="0.768" x2="5.791" y2="-0.768" width="0.12" layer="21"/>
<wire x1="5.831" y1="0.533" x2="5.831" y2="-0.533" width="0.12" layer="21"/>
<circle x="1.75" y="-0" radius="4.12" width="0.12" layer="21"/>
<wire x1="-1.6768" y1="1.7475" x2="-0.8768" y2="1.7475" width="0.1" layer="51"/>
<wire x1="-1.2768" y1="2.1475" x2="-1.2768" y2="1.3475" width="0.1" layer="51"/>
<circle x="1.75" y="-0" radius="4" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="square" first="yes"/>
<pad name="2" x="3.5" y="-0" drill="0.8" diameter="1.7" shape="round"/>
<text x="-3.6197" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-3.6197" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="C_Disc_D5.0mm_W2.5mm_P5.00mm">
<description>Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-0.12" y1="1.37" x2="-0.12" y2="1.055" width="0.12" layer="21"/>
<wire x1="-0.12" y1="1.37" x2="5.12" y2="1.37" width="0.12" layer="21"/>
<wire x1="-0.12" y1="-1.055" x2="-0.12" y2="-1.37" width="0.12" layer="21"/>
<wire x1="-0.12" y1="-1.37" x2="5.12" y2="-1.37" width="0.12" layer="21"/>
<wire x1="5.12" y1="1.37" x2="5.12" y2="1.055" width="0.12" layer="21"/>
<wire x1="5.12" y1="-1.055" x2="5.12" y2="-1.37" width="0.12" layer="21"/>
<wire x1="0" y1="1.25" x2="-0" y2="-1.25" width="0.1" layer="51"/>
<wire x1="-0" y1="-1.25" x2="5" y2="-1.25" width="0.1" layer="51"/>
<wire x1="5" y1="1.25" x2="0" y2="1.25" width="0.1" layer="51"/>
<wire x1="5" y1="-1.25" x2="5" y2="1.25" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="round" first="yes"/>
<pad name="2" x="5" y="-0" drill="0.8" diameter="1.7" shape="round"/>
<text x="-1.975" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-1.975" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="D_DO-41_SOD81_P10.16mm_Horizontal">
<description>Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="1.34" y1="0" x2="2.36" y2="0" width="0.12" layer="21"/>
<wire x1="2.36" y1="1.47" x2="2.36" y2="-1.47" width="0.12" layer="21"/>
<wire x1="2.36" y1="-1.47" x2="7.8" y2="-1.47" width="0.12" layer="21"/>
<wire x1="3.14" y1="1.47" x2="3.14" y2="-1.47" width="0.12" layer="21"/>
<wire x1="3.26" y1="1.47" x2="3.26" y2="-1.47" width="0.12" layer="21"/>
<wire x1="3.38" y1="1.47" x2="3.38" y2="-1.47" width="0.12" layer="21"/>
<wire x1="7.8" y1="1.47" x2="2.36" y2="1.47" width="0.12" layer="21"/>
<wire x1="7.8" y1="-1.47" x2="7.8" y2="1.47" width="0.12" layer="21"/>
<wire x1="8.82" y1="0" x2="7.8" y2="0" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="2.48" y2="0" width="0.1" layer="51"/>
<wire x1="2.48" y1="1.35" x2="2.48" y2="-1.35" width="0.1" layer="51"/>
<wire x1="2.48" y1="-1.35" x2="7.68" y2="-1.35" width="0.1" layer="51"/>
<wire x1="3.16" y1="1.35" x2="3.16" y2="-1.35" width="0.1" layer="51"/>
<wire x1="3.26" y1="1.35" x2="3.26" y2="-1.35" width="0.1" layer="51"/>
<wire x1="3.36" y1="1.35" x2="3.36" y2="-1.35" width="0.1" layer="51"/>
<wire x1="7.68" y1="1.35" x2="2.48" y2="1.35" width="0.1" layer="51"/>
<wire x1="7.68" y1="-1.35" x2="7.68" y2="1.35" width="0.1" layer="51"/>
<wire x1="10.16" y1="0" x2="7.68" y2="0" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1.1" diameter="2.2" shape="square" first="yes"/>
<pad name="2" x="10.16" y="0" drill="1.1" diameter="2.2" shape="round"/>
<text x="5.08" y="2.525" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="5.08" y="0.625" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="D_DO-15_P12.70mm_Horizontal">
<description>Diode_THT:D_DO-15_P12.70mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="1.44" y1="-0" x2="2.43" y2="-0" width="0.12" layer="21"/>
<wire x1="2.43" y1="1.92" x2="2.43" y2="-1.92" width="0.12" layer="21"/>
<wire x1="2.43" y1="-1.92" x2="10.27" y2="-1.92" width="0.12" layer="21"/>
<wire x1="3.57" y1="1.92" x2="3.57" y2="-1.92" width="0.12" layer="21"/>
<wire x1="3.69" y1="1.92" x2="3.69" y2="-1.92" width="0.12" layer="21"/>
<wire x1="3.81" y1="1.92" x2="3.81" y2="-1.92" width="0.12" layer="21"/>
<wire x1="10.27" y1="1.92" x2="2.43" y2="1.92" width="0.12" layer="21"/>
<wire x1="10.27" y1="-1.92" x2="10.27" y2="1.92" width="0.12" layer="21"/>
<wire x1="11.26" y1="-0" x2="10.27" y2="-0" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="2.55" y2="-0" width="0.1" layer="51"/>
<wire x1="2.55" y1="1.8" x2="2.55" y2="-1.8" width="0.1" layer="51"/>
<wire x1="2.55" y1="-1.8" x2="10.15" y2="-1.8" width="0.1" layer="51"/>
<wire x1="3.59" y1="1.8" x2="3.59" y2="-1.8" width="0.1" layer="51"/>
<wire x1="3.69" y1="1.8" x2="3.69" y2="-1.8" width="0.1" layer="51"/>
<wire x1="3.79" y1="1.8" x2="3.79" y2="-1.8" width="0.1" layer="51"/>
<wire x1="10.15" y1="1.8" x2="2.55" y2="1.8" width="0.1" layer="51"/>
<wire x1="10.15" y1="-1.8" x2="10.15" y2="1.8" width="0.1" layer="51"/>
<wire x1="12.7" y1="-0" x2="10.15" y2="-0" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1.2" diameter="2.4" shape="square" first="yes"/>
<pad name="2" x="12.7" y="-0" drill="1.2" diameter="2.4" shape="round"/>
<text x="-2.375" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-2.375" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="D_DO-34_SOD68_P7.62mm_Horizontal">
<description>Diode_THT:D_DO-34_SOD68_P7.62mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="0.99" y1="0" x2="2.17" y2="0" width="0.12" layer="21"/>
<wire x1="2.17" y1="0.92" x2="2.17" y2="-0.92" width="0.12" layer="21"/>
<wire x1="2.17" y1="-0.92" x2="5.45" y2="-0.92" width="0.12" layer="21"/>
<wire x1="2.626" y1="0.92" x2="2.626" y2="-0.92" width="0.12" layer="21"/>
<wire x1="2.746" y1="0.92" x2="2.746" y2="-0.92" width="0.12" layer="21"/>
<wire x1="2.866" y1="0.92" x2="2.866" y2="-0.92" width="0.12" layer="21"/>
<wire x1="5.45" y1="0.92" x2="2.17" y2="0.92" width="0.12" layer="21"/>
<wire x1="5.45" y1="-0.92" x2="5.45" y2="0.92" width="0.12" layer="21"/>
<wire x1="6.63" y1="0" x2="5.45" y2="0" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="2.29" y2="0" width="0.1" layer="51"/>
<wire x1="2.29" y1="0.8" x2="2.29" y2="-0.8" width="0.1" layer="51"/>
<wire x1="2.29" y1="-0.8" x2="5.33" y2="-0.8" width="0.1" layer="51"/>
<wire x1="2.646" y1="0.8" x2="2.646" y2="-0.8" width="0.1" layer="51"/>
<wire x1="2.746" y1="0.8" x2="2.746" y2="-0.8" width="0.1" layer="51"/>
<wire x1="2.846" y1="0.8" x2="2.846" y2="-0.8" width="0.1" layer="51"/>
<wire x1="5.33" y1="0.8" x2="2.29" y2="0.8" width="0.1" layer="51"/>
<wire x1="5.33" y1="-0.8" x2="5.33" y2="0.8" width="0.1" layer="51"/>
<wire x1="7.62" y1="0" x2="5.33" y2="0" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.75" diameter="1.7" shape="square" first="yes"/>
<pad name="2" x="7.62" y="0" drill="0.75" diameter="1.7" shape="round"/>
<text x="9.555" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="9.555" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="PTC_Radial_P5.10mm">
<description>cloudline:PTC_Radial_P5.10mm (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-1.4" y1="1.8" x2="6.5" y2="1.8" width="0.12" layer="21"/>
<wire x1="-1.4" y1="-1.8" x2="-1.4" y2="1.8" width="0.12" layer="21"/>
<wire x1="6.5" y1="1.8" x2="6.5" y2="-1.8" width="0.12" layer="21"/>
<wire x1="6.5" y1="-1.8" x2="-1.4" y2="-1.8" width="0.12" layer="21"/>
<wire x1="-1.25" y1="1.6" x2="6.35" y2="1.6" width="0.1" layer="51"/>
<wire x1="-1.25" y1="-1.6" x2="-1.25" y2="1.6" width="0.1" layer="51"/>
<wire x1="6.35" y1="1.6" x2="6.35" y2="-1.6" width="0.1" layer="51"/>
<wire x1="6.35" y1="-1.6" x2="-1.25" y2="-1.6" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.8" shape="square" first="yes"/>
<pad name="2" x="5.1" y="0" drill="0.8" diameter="1.8" shape="round"/>
<text x="2.55" y="2.975" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="2.55" y="1.075" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="TerminalBlock_Phoenix_MKDS-1_5-4-5.08_1x04_P5.08mm_Horizontal_PAD3.3">
<description>TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-4-5.08_1x04_P5.08mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-2.84" y1="-4.16" x2="-2.84" y2="-4.9" width="0.12" layer="21"/>
<wire x1="-2.84" y1="-4.9" x2="-2.34" y2="-4.9" width="0.12" layer="21"/>
<wire x1="-2.6" y1="5.261" x2="-2.6" y2="-4.66" width="0.12" layer="21"/>
<wire x1="-2.6" y1="5.261" x2="17.84" y2="5.261" width="0.12" layer="21"/>
<wire x1="-2.6" y1="2.301" x2="17.84" y2="2.301" width="0.12" layer="21"/>
<wire x1="-2.6" y1="-2.6" x2="17.84" y2="-2.6" width="0.12" layer="21"/>
<wire x1="-2.6" y1="-4.1" x2="17.84" y2="-4.1" width="0.12" layer="21"/>
<wire x1="-2.6" y1="-4.66" x2="17.84" y2="-4.66" width="0.12" layer="21"/>
<wire x1="3.853" y1="-1.023" x2="3.806" y2="-1.069" width="0.12" layer="21"/>
<wire x1="4.046" y1="-1.239" x2="4.011" y2="-1.274" width="0.12" layer="21"/>
<wire x1="6.15" y1="1.275" x2="6.115" y2="1.239" width="0.12" layer="21"/>
<wire x1="6.355" y1="1.069" x2="6.308" y2="1.023" width="0.12" layer="21"/>
<wire x1="8.933" y1="-1.023" x2="8.886" y2="-1.069" width="0.12" layer="21"/>
<wire x1="9.126" y1="-1.239" x2="9.091" y2="-1.274" width="0.12" layer="21"/>
<wire x1="11.23" y1="1.275" x2="11.195" y2="1.239" width="0.12" layer="21"/>
<wire x1="11.435" y1="1.069" x2="11.388" y2="1.023" width="0.12" layer="21"/>
<wire x1="14.013" y1="-1.023" x2="13.966" y2="-1.069" width="0.12" layer="21"/>
<wire x1="14.206" y1="-1.239" x2="14.171" y2="-1.274" width="0.12" layer="21"/>
<wire x1="16.31" y1="1.275" x2="16.275" y2="1.239" width="0.12" layer="21"/>
<wire x1="16.515" y1="1.069" x2="16.468" y2="1.023" width="0.12" layer="21"/>
<wire x1="17.84" y1="5.261" x2="17.84" y2="-4.66" width="0.12" layer="21"/>
<wire x1="-1.5354" y1="-0.683" x2="-1.535" y2="0.684" width="0.12" layer="21" curve="-48.0002"/>
<wire x1="-0.683" y1="1.5354" x2="0.684" y2="1.535" width="0.12" layer="21" curve="-48.0002"/>
<wire x1="0.0288" y1="-1.6803" x2="-0.684" y2="-1.535" width="0.12" layer="21" curve="-25"/>
<wire x1="0.6833" y1="-1.5348" x2="0" y2="-1.68" width="0.12" layer="21" curve="-24.0001"/>
<wire x1="1.5354" y1="0.683" x2="1.535" y2="-0.684" width="0.12" layer="21" curve="-48.0002"/>
<circle x="5.08" y="0" radius="1.68" width="0.12" layer="21"/>
<circle x="10.16" y="0" radius="1.68" width="0.12" layer="21"/>
<circle x="15.24" y="0" radius="1.68" width="0.12" layer="21"/>
<wire x1="-2.54" y1="5.2" x2="17.78" y2="5.2" width="0.1" layer="51"/>
<wire x1="-2.54" y1="2.3" x2="17.78" y2="2.3" width="0.1" layer="51"/>
<wire x1="-2.54" y1="-2.6" x2="17.78" y2="-2.6" width="0.1" layer="51"/>
<wire x1="-2.54" y1="-4.1" x2="-2.54" y2="5.2" width="0.1" layer="51"/>
<wire x1="-2.54" y1="-4.1" x2="17.78" y2="-4.1" width="0.1" layer="51"/>
<wire x1="-2.04" y1="-4.6" x2="-2.54" y2="-4.1" width="0.1" layer="51"/>
<wire x1="0.955" y1="1.138" x2="-1.138" y2="-0.955" width="0.1" layer="51"/>
<wire x1="1.138" y1="0.955" x2="-0.955" y2="-1.138" width="0.1" layer="51"/>
<wire x1="6.035" y1="1.138" x2="3.943" y2="-0.955" width="0.1" layer="51"/>
<wire x1="6.218" y1="0.955" x2="4.126" y2="-1.138" width="0.1" layer="51"/>
<wire x1="11.115" y1="1.138" x2="9.023" y2="-0.955" width="0.1" layer="51"/>
<wire x1="11.298" y1="0.955" x2="9.206" y2="-1.138" width="0.1" layer="51"/>
<wire x1="16.195" y1="1.138" x2="14.103" y2="-0.955" width="0.1" layer="51"/>
<wire x1="16.378" y1="0.955" x2="14.286" y2="-1.138" width="0.1" layer="51"/>
<wire x1="17.78" y1="5.2" x2="17.78" y2="-4.6" width="0.1" layer="51"/>
<wire x1="17.78" y1="-4.6" x2="-2.04" y2="-4.6" width="0.1" layer="51"/>
<circle x="0" y="0" radius="1.5" width="0.1" layer="51"/>
<circle x="5.08" y="0" radius="1.5" width="0.1" layer="51"/>
<circle x="10.16" y="0" radius="1.5" width="0.1" layer="51"/>
<circle x="15.24" y="0" radius="1.5" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1.3" diameter="3.3" shape="square" first="yes"/>
<pad name="2" x="5.08" y="0" drill="1.3" diameter="3.3" shape="round"/>
<pad name="3" x="10.16" y="0" drill="1.3" diameter="3.3" shape="round"/>
<pad name="4" x="15.24" y="0" drill="1.3" diameter="3.3" shape="round"/>
<text x="19.205" y="0.305" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="19.205" y="-1.595" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="PinHeader_1x05_P2.54mm_Vertical">
<description>Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-1.33" y1="1.33" x2="-0" y2="1.33" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-0" x2="-1.33" y2="1.33" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-1.27" x2="-1.33" y2="-11.49" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-1.27" x2="1.33" y2="-1.27" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-11.49" x2="1.33" y2="-11.49" width="0.12" layer="21"/>
<wire x1="1.33" y1="-1.27" x2="1.33" y2="-11.49" width="0.12" layer="21"/>
<wire x1="-1.27" y1="0.635" x2="-0.635" y2="1.27" width="0.1" layer="51"/>
<wire x1="-1.27" y1="-11.43" x2="-1.27" y2="0.635" width="0.1" layer="51"/>
<wire x1="-0.635" y1="1.27" x2="1.27" y2="1.27" width="0.1" layer="51"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-11.43" width="0.1" layer="51"/>
<wire x1="1.27" y1="-11.43" x2="-1.27" y2="-11.43" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1" diameter="1.9" shape="square" first="yes"/>
<pad name="2" x="0" y="-2.54" drill="1" diameter="1.9" shape="round"/>
<pad name="3" x="0" y="-5.08" drill="1" diameter="1.9" shape="round"/>
<pad name="4" x="0" y="-7.62" drill="1" diameter="1.9" shape="round"/>
<pad name="5" x="0" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<text x="-2.725" y="-5.075" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-2.725" y="-6.975" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="PinHeader_1x04_P2.54mm_Vertical">
<description>Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-1.33" y1="1.33" x2="-0" y2="1.33" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-0" x2="-1.33" y2="1.33" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-1.27" x2="-1.33" y2="-8.95" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-1.27" x2="1.33" y2="-1.27" width="0.12" layer="21"/>
<wire x1="-1.33" y1="-8.95" x2="1.33" y2="-8.95" width="0.12" layer="21"/>
<wire x1="1.33" y1="-1.27" x2="1.33" y2="-8.95" width="0.12" layer="21"/>
<wire x1="-1.27" y1="0.635" x2="-0.635" y2="1.27" width="0.1" layer="51"/>
<wire x1="-1.27" y1="-8.89" x2="-1.27" y2="0.635" width="0.1" layer="51"/>
<wire x1="-0.635" y1="1.27" x2="1.27" y2="1.27" width="0.1" layer="51"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-8.89" width="0.1" layer="51"/>
<wire x1="1.27" y1="-8.89" x2="-1.27" y2="-8.89" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1" diameter="1.9" shape="square" first="yes"/>
<pad name="2" x="0" y="-2.54" drill="1" diameter="1.9" shape="round"/>
<pad name="3" x="0" y="-5.08" drill="1" diameter="1.9" shape="round"/>
<pad name="4" x="0" y="-7.62" drill="1" diameter="1.9" shape="round"/>
<text x="0" y="-10.825" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="0" y="-12.725" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="TO-92_Inline_Wide">
<description>Package_TO_SOT_THT:TO-92_Inline_Wide (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="0.74" y1="-1.85" x2="4.34" y2="-1.85" width="0.12" layer="21"/>
<wire x1="0.1836" y1="1.0988" x2="2.54" y2="2.6" width="0.12" layer="21" curve="-64.9999"/>
<wire x1="0.74" y1="-1.85" x2="0.2158" y2="-1.1228" width="0.12" layer="21" curve="-20.0001"/>
<wire x1="2.54" y1="2.6" x2="4.8964" y2="1.0988" width="0.12" layer="21" curve="-65.0001"/>
<wire x1="4.8642" y1="-1.1228" x2="4.34" y2="-1.85" width="0.12" layer="21" curve="-20.0001"/>
<wire x1="0.8" y1="-1.75" x2="4.3" y2="-1.75" width="0.1" layer="51"/>
<wire x1="0.7864" y1="-1.7536" x2="2.54" y2="2.48" width="0.1" layer="51" curve="-135"/>
<wire x1="2.54" y1="2.48" x2="4.2936" y2="-1.7536" width="0.1" layer="51" curve="-135"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="square" first="yes"/>
<pad name="2" x="2.54" y="0" drill="0.8" diameter="1.7" shape="round"/>
<pad name="3" x="5.08" y="0" drill="0.8" diameter="1.7" shape="round"/>
<text x="2.54" y="3.655" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="2.54" y="1.755" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical">
<description>Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="1.37" y1="0" x2="1.44" y2="0" width="0.12" layer="21"/>
<circle x="0" y="0" radius="1.37" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="2.54" y2="0" width="0.1" layer="51"/>
<circle x="0" y="0" radius="1.25" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="round" first="yes"/>
<pad name="2" x="2.54" y="0" drill="0.8" diameter="1.7" shape="round"/>
<text x="-2.925" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-2.925" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal">
<description>Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="0.54" y1="1.37" x2="7.08" y2="1.37" width="0.12" layer="21"/>
<wire x1="0.54" y1="1.04" x2="0.54" y2="1.37" width="0.12" layer="21"/>
<wire x1="0.54" y1="-1.04" x2="0.54" y2="-1.37" width="0.12" layer="21"/>
<wire x1="0.54" y1="-1.37" x2="7.08" y2="-1.37" width="0.12" layer="21"/>
<wire x1="7.08" y1="1.37" x2="7.08" y2="1.04" width="0.12" layer="21"/>
<wire x1="7.08" y1="-1.37" x2="7.08" y2="-1.04" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="0.66" y2="0" width="0.1" layer="51"/>
<wire x1="0.66" y1="1.25" x2="0.66" y2="-1.25" width="0.1" layer="51"/>
<wire x1="0.66" y1="-1.25" x2="6.96" y2="-1.25" width="0.1" layer="51"/>
<wire x1="6.96" y1="1.25" x2="0.66" y2="1.25" width="0.1" layer="51"/>
<wire x1="6.96" y1="-1.25" x2="6.96" y2="1.25" width="0.1" layer="51"/>
<wire x1="7.62" y1="0" x2="6.96" y2="0" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="round" first="yes"/>
<pad name="2" x="7.62" y="0" drill="0.8" diameter="1.7" shape="round"/>
<text x="3.81" y="2.425" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="3.81" y="0.525" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal">
<description>Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="1.04" y1="-0" x2="1.81" y2="-0" width="0.12" layer="21"/>
<wire x1="1.81" y1="1.37" x2="1.81" y2="-1.37" width="0.12" layer="21"/>
<wire x1="1.81" y1="-1.37" x2="8.35" y2="-1.37" width="0.12" layer="21"/>
<wire x1="8.35" y1="1.37" x2="1.81" y2="1.37" width="0.12" layer="21"/>
<wire x1="8.35" y1="-1.37" x2="8.35" y2="1.37" width="0.12" layer="21"/>
<wire x1="9.12" y1="-0" x2="8.35" y2="-0" width="0.12" layer="21"/>
<wire x1="0" y1="0" x2="1.93" y2="-0" width="0.1" layer="51"/>
<wire x1="1.93" y1="1.25" x2="1.93" y2="-1.25" width="0.1" layer="51"/>
<wire x1="1.93" y1="-1.25" x2="8.23" y2="-1.25" width="0.1" layer="51"/>
<wire x1="8.23" y1="1.25" x2="1.93" y2="1.25" width="0.1" layer="51"/>
<wire x1="8.23" y1="-1.25" x2="8.23" y2="1.25" width="0.1" layer="51"/>
<wire x1="10.16" y1="-0" x2="8.23" y2="-0" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="0.8" diameter="1.7" shape="round" first="yes"/>
<pad name="2" x="10.16" y="-0" drill="0.8" diameter="1.7" shape="round"/>
<text x="-1.975" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-1.975" y="-1.9" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="Arduino_Nano">
<description>Module:Arduino_Nano (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-1.4" y1="3.94" x2="-1.4" y2="1.27" width="0.12" layer="21"/>
<wire x1="-1.4" y1="-1.27" x2="-1.4" y2="-39.5" width="0.12" layer="21"/>
<wire x1="-1.4" y1="-39.5" x2="3.68" y2="-39.5" width="0.12" layer="21"/>
<wire x1="1.27" y1="1.27" x2="-1.4" y2="1.27" width="0.12" layer="21"/>
<wire x1="1.27" y1="-1.27" x2="-1.4" y2="-1.27" width="0.12" layer="21"/>
<wire x1="1.27" y1="-1.27" x2="1.27" y2="1.27" width="0.12" layer="21"/>
<wire x1="1.27" y1="-1.27" x2="1.27" y2="-36.83" width="0.12" layer="21"/>
<wire x1="1.27" y1="-36.83" x2="-1.4" y2="-36.83" width="0.12" layer="21"/>
<wire x1="3.68" y1="-31.62" x2="11.56" y2="-31.62" width="0.12" layer="21"/>
<wire x1="13.97" y1="1.27" x2="13.97" y2="-36.83" width="0.12" layer="21"/>
<wire x1="13.97" y1="1.27" x2="16.64" y2="1.27" width="0.12" layer="21"/>
<wire x1="13.97" y1="-36.83" x2="16.64" y2="-36.83" width="0.12" layer="21"/>
<wire x1="16.64" y1="3.94" x2="-1.4" y2="3.94" width="0.12" layer="21"/>
<wire x1="16.64" y1="-39.5" x2="11.56" y2="-39.5" width="0.12" layer="21"/>
<wire x1="16.64" y1="-39.5" x2="16.64" y2="3.94" width="0.12" layer="21"/>
<wire x1="-1.27" y1="2.54" x2="0" y2="3.81" width="0.1" layer="51"/>
<wire x1="-1.27" y1="-39.37" x2="-1.27" y2="2.54" width="0.1" layer="51"/>
<wire x1="0" y1="3.81" x2="16.51" y2="3.81" width="0.1" layer="51"/>
<wire x1="3.81" y1="-31.75" x2="11.43" y2="-31.75" width="0.1" layer="51"/>
<wire x1="3.81" y1="-41.91" x2="3.81" y2="-31.75" width="0.1" layer="51"/>
<wire x1="11.43" y1="-31.75" x2="11.43" y2="-41.91" width="0.1" layer="51"/>
<wire x1="11.43" y1="-41.91" x2="3.81" y2="-41.91" width="0.1" layer="51"/>
<wire x1="16.51" y1="3.81" x2="16.51" y2="-39.37" width="0.1" layer="51"/>
<wire x1="16.51" y1="-39.37" x2="-1.27" y2="-39.37" width="0.1" layer="51"/>
<pad name="1" x="0" y="0" drill="1" diameter="1.9" shape="square" first="yes"/>
<pad name="2" x="-0" y="-2.54" drill="1" diameter="1.9" shape="round"/>
<pad name="3" x="-0" y="-5.08" drill="1" diameter="1.9" shape="round"/>
<pad name="4" x="-0" y="-7.62" drill="1" diameter="1.9" shape="round"/>
<pad name="5" x="-0" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="6" x="-0" y="-12.7" drill="1" diameter="1.9" shape="round"/>
<pad name="7" x="-0" y="-15.24" drill="1" diameter="1.9" shape="round"/>
<pad name="8" x="-0" y="-17.78" drill="1" diameter="1.9" shape="round"/>
<pad name="9" x="-0" y="-20.32" drill="1" diameter="1.9" shape="round"/>
<pad name="10" x="-0" y="-22.86" drill="1" diameter="1.9" shape="round"/>
<pad name="11" x="-0" y="-25.4" drill="1" diameter="1.9" shape="round"/>
<pad name="12" x="-0" y="-27.94" drill="1" diameter="1.9" shape="round"/>
<pad name="13" x="-0" y="-30.48" drill="1" diameter="1.9" shape="round"/>
<pad name="14" x="-0" y="-33.02" drill="1" diameter="1.9" shape="round"/>
<pad name="15" x="-0" y="-35.56" drill="1" diameter="1.9" shape="round"/>
<pad name="16" x="15.24" y="-35.56" drill="1" diameter="1.9" shape="round"/>
<pad name="17" x="15.24" y="-33.02" drill="1" diameter="1.9" shape="round"/>
<pad name="18" x="15.24" y="-30.48" drill="1" diameter="1.9" shape="round"/>
<pad name="19" x="15.24" y="-27.94" drill="1" diameter="1.9" shape="round"/>
<pad name="20" x="15.24" y="-25.4" drill="1" diameter="1.9" shape="round"/>
<pad name="21" x="15.24" y="-22.86" drill="1" diameter="1.9" shape="round"/>
<pad name="22" x="15.24" y="-20.32" drill="1" diameter="1.9" shape="round"/>
<pad name="23" x="15.24" y="-17.78" drill="1" diameter="1.9" shape="round"/>
<pad name="24" x="15.24" y="-15.24" drill="1" diameter="1.9" shape="round"/>
<pad name="25" x="15.24" y="-12.7" drill="1" diameter="1.9" shape="round"/>
<pad name="26" x="15.24" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="27" x="15.24" y="-7.62" drill="1" diameter="1.9" shape="round"/>
<pad name="28" x="15.24" y="-5.08" drill="1" diameter="1.9" shape="round"/>
<pad name="29" x="15.24" y="-2.54" drill="1" diameter="1.9" shape="round"/>
<pad name="30" x="15.24" y="-0" drill="1" diameter="1.9" shape="round"/>
<text x="-2.455" y="-19.05" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-2.455" y="-20.95" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
<package name="SparkFun_BOB-12009">
<description>cloudline:SparkFun_BOB-12009 (from KiCad, pads enlarged for a single-sided board)</description>
<wire x1="-1.27" y1="1.27" x2="13.97" y2="1.27" width="0.12" layer="21"/>
<wire x1="-1.27" y1="-11.43" x2="-1.27" y2="1.27" width="0.12" layer="21"/>
<wire x1="13.97" y1="1.27" x2="13.97" y2="-11.43" width="0.12" layer="21"/>
<wire x1="13.97" y1="-11.43" x2="-1.27" y2="-11.43" width="0.12" layer="21"/>
<wire x1="-1.27" y1="1.27" x2="13.97" y2="1.27" width="0.12" layer="51"/>
<wire x1="-1.27" y1="-11.43" x2="-1.27" y2="1.27" width="0.12" layer="51"/>
<wire x1="13.97" y1="1.27" x2="13.97" y2="-11.43" width="0.12" layer="51"/>
<wire x1="13.97" y1="-11.43" x2="-1.27" y2="-11.43" width="0.12" layer="51"/>
<pad name="GND_HV" x="7.62" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="GND_LV" x="7.62" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="HV" x="5.08" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="HV1" x="0" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="HV2" x="2.54" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="HV3" x="10.16" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="HV4" x="12.7" y="0" drill="1" diameter="1.9" shape="round"/>
<pad name="LV" x="5.08" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="LV1" x="0" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="LV2" x="2.54" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="LV3" x="10.16" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<pad name="LV4" x="12.7" y="-10.16" drill="1" diameter="1.9" shape="round"/>
<text x="6.35" y="2.445" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="6.35" y="0.545" size="1.27" layer="27" align="center">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="SYM_C1">
<pin name="+" x="-7.62" y="0" length="short" visible="pin"/>
<pin name="-" x="7.62" y="0" length="short" visible="pin" rot="R180"/>
<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-2.54" y="1.27" size="1.27" layer="94">+</text>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_C2">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_C3">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_C4">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_C5">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_D1">
<pin name="A" x="-7.62" y="0" length="short" visible="pin"/>
<pin name="K" x="7.62" y="0" length="short" visible="pin" rot="R180"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-5.08" y1="0" x2="-1.27" y2="0" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_D2">
<pin name="A" x="-7.62" y="0" length="short" visible="pin"/>
<pin name="K" x="7.62" y="0" length="short" visible="pin" rot="R180"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-5.08" y1="0" x2="-1.27" y2="0" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_D3">
<pin name="A" x="-7.62" y="0" length="short" visible="pin"/>
<pin name="K" x="7.62" y="0" length="short" visible="pin" rot="R180"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-5.08" y1="0" x2="-1.27" y2="0" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_D4">
<pin name="A" x="-7.62" y="0" length="short" visible="pin"/>
<pin name="K" x="7.62" y="0" length="short" visible="pin" rot="R180"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-5.08" y1="0" x2="-1.27" y2="0" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_F1">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="5.08" y2="1.016" width="0.1524" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_J2">
<pin name="1_+10V" x="-10.16" y="5.08" length="short"/>
<pin name="2_TACH" x="-10.16" y="2.54" length="short"/>
<pin name="3_GND" x="-10.16" y="0" length="short"/>
<pin name="4_PWM" x="-10.16" y="-2.54" length="short"/>
<wire x1="-7.62" y1="7.62" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="7.62" width="0.254" layer="94"/>
<text x="-7.62" y="8.382" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_J3">
<pin name="1_BAK" x="-10.16" y="2.54" length="short"/>
<pin name="2_CON" x="-10.16" y="0" length="short"/>
<pin name="3_PSH" x="-10.16" y="-2.54" length="short"/>
<pin name="4_TRB" x="10.16" y="2.54" length="short" rot="R180"/>
<pin name="5_TRA" x="10.16" y="0" length="short" rot="R180"/>
<wire x1="-7.62" y1="5.08" x2="7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="5.08" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="5.08" width="0.254" layer="94"/>
<text x="-7.62" y="5.842" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_J4">
<pin name="1_SDA" x="-10.16" y="5.08" length="short"/>
<pin name="2_SCL" x="-10.16" y="2.54" length="short"/>
<pin name="3_VCC" x="-10.16" y="0" length="short"/>
<pin name="4_GND" x="-10.16" y="-2.54" length="short"/>
<wire x1="-7.62" y1="7.62" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="7.62" width="0.254" layer="94"/>
<text x="-7.62" y="8.382" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_Q1">
<pin name="S" x="-10.16" y="2.54" length="short"/>
<pin name="G" x="-10.16" y="0" length="short"/>
<pin name="D" x="-10.16" y="-2.54" length="short"/>
<wire x1="-7.62" y1="5.08" x2="7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="5.08" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="5.08" width="0.254" layer="94"/>
<text x="-7.62" y="5.842" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R1">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R2">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R3">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R4">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R5">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R6">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_R7">
<pin name="1" x="-7.62" y="0" length="short" visible="pad"/>
<pin name="2" x="7.62" y="0" length="short" visible="pad" rot="R180"/>
<wire x1="-5.08" y1="1.016" x2="5.08" y2="1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.016" x2="5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="5.08" y1="-1.016" x2="-5.08" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-1.016" x2="-5.08" y2="1.016" width="0.254" layer="94"/>
<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_U1">
<pin name="D1/TX" x="-12.7" y="17.78" length="short"/>
<pin name="D0/RX" x="-12.7" y="15.24" length="short"/>
<pin name="RESET@1" x="-12.7" y="12.7" length="short"/>
<pin name="GND@1" x="-12.7" y="10.16" length="short"/>
<pin name="D2" x="-12.7" y="7.62" length="short"/>
<pin name="D3" x="-12.7" y="5.08" length="short"/>
<pin name="D4" x="-12.7" y="2.54" length="short"/>
<pin name="D5" x="-12.7" y="0" length="short"/>
<pin name="D6" x="-12.7" y="-2.54" length="short"/>
<pin name="D7" x="-12.7" y="-5.08" length="short"/>
<pin name="D8" x="-12.7" y="-7.62" length="short"/>
<pin name="D9" x="-12.7" y="-10.16" length="short"/>
<pin name="D10" x="-12.7" y="-12.7" length="short"/>
<pin name="D11" x="-12.7" y="-15.24" length="short"/>
<pin name="D12" x="-12.7" y="-17.78" length="short"/>
<pin name="D13" x="12.7" y="17.78" length="short" rot="R180"/>
<pin name="3V3" x="12.7" y="15.24" length="short" rot="R180"/>
<pin name="AREF" x="12.7" y="12.7" length="short" rot="R180"/>
<pin name="A0" x="12.7" y="10.16" length="short" rot="R180"/>
<pin name="A1" x="12.7" y="7.62" length="short" rot="R180"/>
<pin name="A2" x="12.7" y="5.08" length="short" rot="R180"/>
<pin name="A3" x="12.7" y="2.54" length="short" rot="R180"/>
<pin name="A4/SDA" x="12.7" y="0" length="short" rot="R180"/>
<pin name="A5/SCL" x="12.7" y="-2.54" length="short" rot="R180"/>
<pin name="A6" x="12.7" y="-5.08" length="short" rot="R180"/>
<pin name="A7" x="12.7" y="-7.62" length="short" rot="R180"/>
<pin name="+5V" x="12.7" y="-10.16" length="short" rot="R180"/>
<pin name="RESET@2" x="12.7" y="-12.7" length="short" rot="R180"/>
<pin name="GND@2" x="12.7" y="-15.24" length="short" rot="R180"/>
<pin name="VIN" x="12.7" y="-17.78" length="short" rot="R180"/>
<wire x1="-10.16" y1="20.32" x2="10.16" y2="20.32" width="0.254" layer="94"/>
<wire x1="10.16" y1="20.32" x2="10.16" y2="-20.32" width="0.254" layer="94"/>
<wire x1="10.16" y1="-20.32" x2="-10.16" y2="-20.32" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-20.32" x2="-10.16" y2="20.32" width="0.254" layer="94"/>
<text x="-10.16" y="21.082" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-22.86" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_U2">
<pin name="GND" x="-10.16" y="2.54" length="short"/>
<pin name="VIN" x="-10.16" y="0" length="short"/>
<pin name="VOUT" x="-10.16" y="-2.54" length="short"/>
<wire x1="-7.62" y1="5.08" x2="7.62" y2="5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="5.08" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="7.62" y1="-5.08" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="5.08" width="0.254" layer="94"/>
<text x="-7.62" y="5.842" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="SYM_U4">
<pin name="HV1" x="-10.16" y="5.08" length="short"/>
<pin name="HV2" x="-10.16" y="2.54" length="short"/>
<pin name="HV3" x="-10.16" y="0" length="short"/>
<pin name="HV4" x="-10.16" y="-2.54" length="short"/>
<pin name="HV" x="-10.16" y="-5.08" length="short"/>
<pin name="GND_HV" x="-10.16" y="-7.62" length="short"/>
<pin name="LV1" x="10.16" y="5.08" length="short" rot="R180"/>
<pin name="LV2" x="10.16" y="2.54" length="short" rot="R180"/>
<pin name="LV3" x="10.16" y="0" length="short" rot="R180"/>
<pin name="LV4" x="10.16" y="-2.54" length="short" rot="R180"/>
<pin name="LV" x="10.16" y="-5.08" length="short" rot="R180"/>
<pin name="GND_LV" x="10.16" y="-7.62" length="short" rot="R180"/>
<wire x1="-7.62" y1="7.62" x2="7.62" y2="7.62" width="0.254" layer="94"/>
<wire x1="7.62" y1="7.62" x2="7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="7.62" y1="-10.16" x2="-7.62" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-10.16" x2="-7.62" y2="7.62" width="0.254" layer="94"/>
<text x="-7.62" y="8.382" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-12.7" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="DEV_C1" prefix="C">
<gates><gate name="G$1" symbol="SYM_C1" x="0" y="0"/></gates>
<devices><device name="" package="CP_Radial_D8.0mm_P3.50mm"><connects>
<connect gate="G$1" pin="+" pad="1"/>
<connect gate="G$1" pin="-" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_C2" prefix="C">
<gates><gate name="G$1" symbol="SYM_C2" x="0" y="0"/></gates>
<devices><device name="" package="C_Disc_D5.0mm_W2.5mm_P5.00mm"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_C3" prefix="C">
<gates><gate name="G$1" symbol="SYM_C3" x="0" y="0"/></gates>
<devices><device name="" package="C_Disc_D5.0mm_W2.5mm_P5.00mm"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_C4" prefix="C">
<gates><gate name="G$1" symbol="SYM_C4" x="0" y="0"/></gates>
<devices><device name="" package="C_Disc_D5.0mm_W2.5mm_P5.00mm"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_C5" prefix="C">
<gates><gate name="G$1" symbol="SYM_C5" x="0" y="0"/></gates>
<devices><device name="" package="C_Disc_D5.0mm_W2.5mm_P5.00mm"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_D1" prefix="D">
<gates><gate name="G$1" symbol="SYM_D1" x="0" y="0"/></gates>
<devices><device name="" package="D_DO-41_SOD81_P10.16mm_Horizontal"><connects>
<connect gate="G$1" pin="K" pad="1"/>
<connect gate="G$1" pin="A" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_D2" prefix="D">
<gates><gate name="G$1" symbol="SYM_D2" x="0" y="0"/></gates>
<devices><device name="" package="D_DO-15_P12.70mm_Horizontal"><connects>
<connect gate="G$1" pin="K" pad="1"/>
<connect gate="G$1" pin="A" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_D3" prefix="D">
<gates><gate name="G$1" symbol="SYM_D3" x="0" y="0"/></gates>
<devices><device name="" package="D_DO-34_SOD68_P7.62mm_Horizontal"><connects>
<connect gate="G$1" pin="K" pad="1"/>
<connect gate="G$1" pin="A" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_D4" prefix="D">
<gates><gate name="G$1" symbol="SYM_D4" x="0" y="0"/></gates>
<devices><device name="" package="D_DO-34_SOD68_P7.62mm_Horizontal"><connects>
<connect gate="G$1" pin="K" pad="1"/>
<connect gate="G$1" pin="A" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_F1" prefix="F">
<gates><gate name="G$1" symbol="SYM_F1" x="0" y="0"/></gates>
<devices><device name="" package="PTC_Radial_P5.10mm"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_J2" prefix="J">
<gates><gate name="G$1" symbol="SYM_J2" x="0" y="0"/></gates>
<devices><device name="" package="TerminalBlock_Phoenix_MKDS-1_5-4-5.08_1x04_P5.08mm_Horizontal_PAD3.3"><connects>
<connect gate="G$1" pin="1_+10V" pad="1"/>
<connect gate="G$1" pin="2_TACH" pad="2"/>
<connect gate="G$1" pin="3_GND" pad="3"/>
<connect gate="G$1" pin="4_PWM" pad="4"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_J3" prefix="J">
<gates><gate name="G$1" symbol="SYM_J3" x="0" y="0"/></gates>
<devices><device name="" package="PinHeader_1x05_P2.54mm_Vertical"><connects>
<connect gate="G$1" pin="1_BAK" pad="1"/>
<connect gate="G$1" pin="2_CON" pad="2"/>
<connect gate="G$1" pin="3_PSH" pad="3"/>
<connect gate="G$1" pin="4_TRB" pad="4"/>
<connect gate="G$1" pin="5_TRA" pad="5"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_J4" prefix="J">
<gates><gate name="G$1" symbol="SYM_J4" x="0" y="0"/></gates>
<devices><device name="" package="PinHeader_1x04_P2.54mm_Vertical"><connects>
<connect gate="G$1" pin="1_SDA" pad="1"/>
<connect gate="G$1" pin="2_SCL" pad="2"/>
<connect gate="G$1" pin="3_VCC" pad="3"/>
<connect gate="G$1" pin="4_GND" pad="4"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_Q1" prefix="Q">
<gates><gate name="G$1" symbol="SYM_Q1" x="0" y="0"/></gates>
<devices><device name="" package="TO-92_Inline_Wide"><connects>
<connect gate="G$1" pin="S" pad="1"/>
<connect gate="G$1" pin="G" pad="2"/>
<connect gate="G$1" pin="D" pad="3"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R1" prefix="R">
<gates><gate name="G$1" symbol="SYM_R1" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R2" prefix="R">
<gates><gate name="G$1" symbol="SYM_R2" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R3" prefix="R">
<gates><gate name="G$1" symbol="SYM_R3" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R4" prefix="R">
<gates><gate name="G$1" symbol="SYM_R4" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R5" prefix="R">
<gates><gate name="G$1" symbol="SYM_R5" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R6" prefix="R">
<gates><gate name="G$1" symbol="SYM_R6" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_R7" prefix="R">
<gates><gate name="G$1" symbol="SYM_R7" x="0" y="0"/></gates>
<devices><device name="" package="R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"><connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_U1" prefix="U">
<gates><gate name="G$1" symbol="SYM_U1" x="0" y="0"/></gates>
<devices><device name="" package="Arduino_Nano"><connects>
<connect gate="G$1" pin="D1/TX" pad="1"/>
<connect gate="G$1" pin="D0/RX" pad="2"/>
<connect gate="G$1" pin="RESET@1" pad="3"/>
<connect gate="G$1" pin="GND@1" pad="4"/>
<connect gate="G$1" pin="D2" pad="5"/>
<connect gate="G$1" pin="D3" pad="6"/>
<connect gate="G$1" pin="D4" pad="7"/>
<connect gate="G$1" pin="D5" pad="8"/>
<connect gate="G$1" pin="D6" pad="9"/>
<connect gate="G$1" pin="D7" pad="10"/>
<connect gate="G$1" pin="D8" pad="11"/>
<connect gate="G$1" pin="D9" pad="12"/>
<connect gate="G$1" pin="D10" pad="13"/>
<connect gate="G$1" pin="D11" pad="14"/>
<connect gate="G$1" pin="D12" pad="15"/>
<connect gate="G$1" pin="D13" pad="16"/>
<connect gate="G$1" pin="3V3" pad="17"/>
<connect gate="G$1" pin="AREF" pad="18"/>
<connect gate="G$1" pin="A0" pad="19"/>
<connect gate="G$1" pin="A1" pad="20"/>
<connect gate="G$1" pin="A2" pad="21"/>
<connect gate="G$1" pin="A3" pad="22"/>
<connect gate="G$1" pin="A4/SDA" pad="23"/>
<connect gate="G$1" pin="A5/SCL" pad="24"/>
<connect gate="G$1" pin="A6" pad="25"/>
<connect gate="G$1" pin="A7" pad="26"/>
<connect gate="G$1" pin="+5V" pad="27"/>
<connect gate="G$1" pin="RESET@2" pad="28"/>
<connect gate="G$1" pin="GND@2" pad="29"/>
<connect gate="G$1" pin="VIN" pad="30"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_U2" prefix="U">
<gates><gate name="G$1" symbol="SYM_U2" x="0" y="0"/></gates>
<devices><device name="" package="TO-92_Inline_Wide"><connects>
<connect gate="G$1" pin="GND" pad="1"/>
<connect gate="G$1" pin="VIN" pad="2"/>
<connect gate="G$1" pin="VOUT" pad="3"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
<deviceset name="DEV_U4" prefix="U">
<gates><gate name="G$1" symbol="SYM_U4" x="0" y="0"/></gates>
<devices><device name="" package="SparkFun_BOB-12009"><connects>
<connect gate="G$1" pin="HV1" pad="HV1"/>
<connect gate="G$1" pin="HV2" pad="HV2"/>
<connect gate="G$1" pin="HV3" pad="HV3"/>
<connect gate="G$1" pin="HV4" pad="HV4"/>
<connect gate="G$1" pin="HV" pad="HV"/>
<connect gate="G$1" pin="GND_HV" pad="GND_HV"/>
<connect gate="G$1" pin="LV1" pad="LV1"/>
<connect gate="G$1" pin="LV2" pad="LV2"/>
<connect gate="G$1" pin="LV3" pad="LV3"/>
<connect gate="G$1" pin="LV4" pad="LV4"/>
<connect gate="G$1" pin="LV" pad="LV"/>
<connect gate="G$1" pin="GND_LV" pad="GND_LV"/>
</connects><technologies><technology name=""/></technologies></device></devices>
</deviceset>
</devicesets>
</library>
</libraries>
<classes>
<class number="0" name="default" width="0.5" drill="0"><clearance class="0" value="0.6"/></class>
<class number="1" name="power" width="0.8" drill="0"><clearance class="1" value="0.6"/><clearance class="0" value="0.6"/></class>
</classes>
<parts>
<part name="C1" library="cloudline" deviceset="DEV_C1" device="" value="220uF 25V"/>
<part name="C2" library="cloudline" deviceset="DEV_C2" device="" value="100nF"/>
<part name="C3" library="cloudline" deviceset="DEV_C3" device="" value="1uF"/>
<part name="C4" library="cloudline" deviceset="DEV_C4" device="" value="1uF"/>
<part name="C5" library="cloudline" deviceset="DEV_C5" device="" value="10nF"/>
<part name="D1" library="cloudline" deviceset="DEV_D1" device="" value="1N5817"/>
<part name="D2" library="cloudline" deviceset="DEV_D2" device="" value="P6KE13A"/>
<part name="D3" library="cloudline" deviceset="DEV_D3" device="" value="BAT85"/>
<part name="D4" library="cloudline" deviceset="DEV_D4" device="" value="BAT85"/>
<part name="F1" library="cloudline" deviceset="DEV_F1" device="" value="PTC 0.2A MF-R020"/>
<part name="J2" library="cloudline" deviceset="DEV_J2" device="" value="Fan: 1 +10V 2 TACH 3 GND 4 PWM"/>
<part name="J3" library="cloudline" deviceset="DEV_J3" device="" value="OLED keys: BAK CON PSH TRB TRA"/>
<part name="J4" library="cloudline" deviceset="DEV_J4" device="" value="OLED: SDA SCL VCC GND"/>
<part name="Q1" library="cloudline" deviceset="DEV_Q1" device="" value="2N7000"/>
<part name="R1" library="cloudline" deviceset="DEV_R1" device="" value="2.2k"/>
<part name="R2" library="cloudline" deviceset="DEV_R2" device="" value="2.2k"/>
<part name="R3" library="cloudline" deviceset="DEV_R3" device="" value="220"/>
<part name="R4" library="cloudline" deviceset="DEV_R4" device="" value="10k"/>
<part name="R5" library="cloudline" deviceset="DEV_R5" device="" value="4.7k DNP"/>
<part name="R6" library="cloudline" deviceset="DEV_R6" device="" value="10k"/>
<part name="R7" library="cloudline" deviceset="DEV_R7" device="" value="1k"/>
<part name="U1" library="cloudline" deviceset="DEV_U1" device="" value="Arduino Nano (socketed)"/>
<part name="U2" library="cloudline" deviceset="DEV_U2" device="" value="MCP1700-3302E"/>
<part name="U4" library="cloudline" deviceset="DEV_U4" device="" value="SparkFun BOB-12009"/>
</parts>
<sheets>
<sheet>
<plain>
<text x="0" y="-10.16" size="2.54" layer="97">Cloudline AI-CLS6 controller, Rev B. Nets are joined by their labels. Generated from the KiCad board by make_eagle.py; the drawn schematic is hardware/schematic.png.</text>
</plain>
<instances>
<instance part="C1" gate="G$1" x="160.02" y="142.24"/>
<instance part="C2" gate="G$1" x="160.02" y="127"/>
<instance part="C3" gate="G$1" x="78.74" y="30.48"/>
<instance part="C4" gate="G$1" x="124.46" y="10.16"/>
<instance part="C5" gate="G$1" x="160.02" y="76.2"/>
<instance part="D1" gate="G$1" x="210.82" y="157.48"/>
<instance part="D2" gate="G$1" x="160.02" y="157.48"/>
<instance part="D3" gate="G$1" x="160.02" y="106.68"/>
<instance part="D4" gate="G$1" x="160.02" y="91.44"/>
<instance part="F1" gate="G$1" x="261.62" y="157.48"/>
<instance part="J2" gate="G$1" x="289.56" y="121.92"/>
<instance part="J3" gate="G$1" x="12.7" y="147.32"/>
<instance part="J4" gate="G$1" x="220.98" y="30.48"/>
<instance part="Q1" gate="G$1" x="241.3" y="129.54"/>
<instance part="R1" gate="G$1" x="154.94" y="45.72"/>
<instance part="R2" gate="G$1" x="154.94" y="30.48"/>
<instance part="R3" gate="G$1" x="195.58" y="129.54"/>
<instance part="R4" gate="G$1" x="195.58" y="114.3"/>
<instance part="R5" gate="G$1" x="226.06" y="109.22"/>
<instance part="R6" gate="G$1" x="195.58" y="99.06"/>
<instance part="R7" gate="G$1" x="226.06" y="91.44"/>
<instance part="U1" gate="G$1" x="60.96" y="101.6"/>
<instance part="U2" gate="G$1" x="124.46" y="30.48"/>
<instance part="U4" gate="G$1" x="190.5" y="76.2"/>
</instances>
<nets>
<net name="+10V_FAN" class="1">
<segment>
<pinref part="F1" gate="G$1" pin="1"/>
<wire x1="254" y1="157.48" x2="248.92" y2="157.48" width="0.1524" layer="91"/>
<label x="248.92" y="157.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="J2" gate="G$1" pin="1_+10V"/>
<wire x1="279.4" y1="127" x2="274.32" y2="127" width="0.1524" layer="91"/>
<label x="274.32" y="127" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R5" gate="G$1" pin="2"/>
<wire x1="233.68" y1="109.22" x2="238.76" y2="109.22" width="0.1524" layer="91"/>
<label x="238.76" y="109.22" size="1.27" layer="95"/>
</segment>
</net>
<net name="+10V_P" class="1">
<segment>
<pinref part="C1" gate="G$1" pin="+"/>
<wire x1="152.4" y1="142.24" x2="147.32" y2="142.24" width="0.1524" layer="91"/>
<label x="147.32" y="142.24" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="C2" gate="G$1" pin="1"/>
<wire x1="152.4" y1="127" x2="147.32" y2="127" width="0.1524" layer="91"/>
<label x="147.32" y="127" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="D1" gate="G$1" pin="K"/>
<wire x1="218.44" y1="157.48" x2="223.52" y2="157.48" width="0.1524" layer="91"/>
<label x="223.52" y="157.48" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="D2" gate="G$1" pin="K"/>
<wire x1="167.64" y1="157.48" x2="172.72" y2="157.48" width="0.1524" layer="91"/>
<label x="172.72" y="157.48" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="VIN"/>
<wire x1="73.66" y1="83.82" x2="78.74" y2="83.82" width="0.1524" layer="91"/>
<label x="78.74" y="83.82" size="1.27" layer="95"/>
</segment>
</net>
<net name="+3V3" class="0">
<segment>
<pinref part="C4" gate="G$1" pin="1"/>
<wire x1="116.84" y1="10.16" x2="111.76" y2="10.16" width="0.1524" layer="91"/>
<label x="111.76" y="10.16" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="J4" gate="G$1" pin="3_VCC"/>
<wire x1="210.82" y1="30.48" x2="205.74" y2="30.48" width="0.1524" layer="91"/>
<label x="205.74" y="30.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R1" gate="G$1" pin="1"/>
<wire x1="147.32" y1="45.72" x2="142.24" y2="45.72" width="0.1524" layer="91"/>
<label x="142.24" y="45.72" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R2" gate="G$1" pin="1"/>
<wire x1="147.32" y1="30.48" x2="142.24" y2="30.48" width="0.1524" layer="91"/>
<label x="142.24" y="30.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U2" gate="G$1" pin="VOUT"/>
<wire x1="114.3" y1="27.94" x2="109.22" y2="27.94" width="0.1524" layer="91"/>
<label x="109.22" y="27.94" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="LV"/>
<wire x1="200.66" y1="71.12" x2="205.74" y2="71.12" width="0.1524" layer="91"/>
<label x="205.74" y="71.12" size="1.27" layer="95"/>
</segment>
</net>
<net name="+5V" class="1">
<segment>
<pinref part="C3" gate="G$1" pin="1"/>
<wire x1="71.12" y1="30.48" x2="66.04" y2="30.48" width="0.1524" layer="91"/>
<label x="66.04" y="30.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="D3" gate="G$1" pin="K"/>
<wire x1="167.64" y1="106.68" x2="172.72" y2="106.68" width="0.1524" layer="91"/>
<label x="172.72" y="106.68" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="R4" gate="G$1" pin="2"/>
<wire x1="203.2" y1="114.3" x2="208.28" y2="114.3" width="0.1524" layer="91"/>
<label x="208.28" y="114.3" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="R6" gate="G$1" pin="2"/>
<wire x1="203.2" y1="99.06" x2="208.28" y2="99.06" width="0.1524" layer="91"/>
<label x="208.28" y="99.06" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="+5V"/>
<wire x1="73.66" y1="91.44" x2="78.74" y2="91.44" width="0.1524" layer="91"/>
<label x="78.74" y="91.44" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U2" gate="G$1" pin="VIN"/>
<wire x1="114.3" y1="30.48" x2="109.22" y2="30.48" width="0.1524" layer="91"/>
<label x="109.22" y="30.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="HV"/>
<wire x1="180.34" y1="71.12" x2="175.26" y2="71.12" width="0.1524" layer="91"/>
<label x="175.26" y="71.12" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="ENC_A" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="5_TRA"/>
<wire x1="22.86" y1="147.32" x2="27.94" y2="147.32" width="0.1524" layer="91"/>
<label x="27.94" y="147.32" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D3"/>
<wire x1="48.26" y1="106.68" x2="43.18" y2="106.68" width="0.1524" layer="91"/>
<label x="43.18" y="106.68" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="ENC_B" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="4_TRB"/>
<wire x1="22.86" y1="149.86" x2="27.94" y2="149.86" width="0.1524" layer="91"/>
<label x="27.94" y="149.86" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D4"/>
<wire x1="48.26" y1="104.14" x2="43.18" y2="104.14" width="0.1524" layer="91"/>
<label x="43.18" y="104.14" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="ENC_SW" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="3_PSH"/>
<wire x1="2.54" y1="144.78" x2="-2.54" y2="144.78" width="0.1524" layer="91"/>
<label x="-2.54" y="144.78" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D5"/>
<wire x1="48.26" y1="101.6" x2="43.18" y2="101.6" width="0.1524" layer="91"/>
<label x="43.18" y="101.6" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="FAN_PWM" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="4_PWM"/>
<wire x1="279.4" y1="119.38" x2="274.32" y2="119.38" width="0.1524" layer="91"/>
<label x="274.32" y="119.38" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="Q1" gate="G$1" pin="D"/>
<wire x1="231.14" y1="127" x2="226.06" y2="127" width="0.1524" layer="91"/>
<label x="226.06" y="127" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R5" gate="G$1" pin="1"/>
<wire x1="218.44" y1="109.22" x2="213.36" y2="109.22" width="0.1524" layer="91"/>
<label x="213.36" y="109.22" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="GATE" class="0">
<segment>
<pinref part="Q1" gate="G$1" pin="G"/>
<wire x1="231.14" y1="129.54" x2="226.06" y2="129.54" width="0.1524" layer="91"/>
<label x="226.06" y="129.54" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R3" gate="G$1" pin="2"/>
<wire x1="203.2" y1="129.54" x2="208.28" y2="129.54" width="0.1524" layer="91"/>
<label x="208.28" y="129.54" size="1.27" layer="95"/>
</segment>
</net>
<net name="GND" class="1">
<segment>
<pinref part="C1" gate="G$1" pin="-"/>
<wire x1="167.64" y1="142.24" x2="172.72" y2="142.24" width="0.1524" layer="91"/>
<label x="172.72" y="142.24" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="C2" gate="G$1" pin="2"/>
<wire x1="167.64" y1="127" x2="172.72" y2="127" width="0.1524" layer="91"/>
<label x="172.72" y="127" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="C3" gate="G$1" pin="2"/>
<wire x1="86.36" y1="30.48" x2="91.44" y2="30.48" width="0.1524" layer="91"/>
<label x="91.44" y="30.48" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="C4" gate="G$1" pin="2"/>
<wire x1="132.08" y1="10.16" x2="137.16" y2="10.16" width="0.1524" layer="91"/>
<label x="137.16" y="10.16" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="C5" gate="G$1" pin="2"/>
<wire x1="167.64" y1="76.2" x2="172.72" y2="76.2" width="0.1524" layer="91"/>
<label x="172.72" y="76.2" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="D2" gate="G$1" pin="A"/>
<wire x1="152.4" y1="157.48" x2="147.32" y2="157.48" width="0.1524" layer="91"/>
<label x="147.32" y="157.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="D4" gate="G$1" pin="A"/>
<wire x1="152.4" y1="91.44" x2="147.32" y2="91.44" width="0.1524" layer="91"/>
<label x="147.32" y="91.44" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="J2" gate="G$1" pin="3_GND"/>
<wire x1="279.4" y1="121.92" x2="274.32" y2="121.92" width="0.1524" layer="91"/>
<label x="274.32" y="121.92" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="J4" gate="G$1" pin="4_GND"/>
<wire x1="210.82" y1="27.94" x2="205.74" y2="27.94" width="0.1524" layer="91"/>
<label x="205.74" y="27.94" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="Q1" gate="G$1" pin="S"/>
<wire x1="231.14" y1="132.08" x2="226.06" y2="132.08" width="0.1524" layer="91"/>
<label x="226.06" y="132.08" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="GND@2"/>
<wire x1="73.66" y1="86.36" x2="78.74" y2="86.36" width="0.1524" layer="91"/>
<label x="78.74" y="86.36" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U2" gate="G$1" pin="GND"/>
<wire x1="114.3" y1="33.02" x2="109.22" y2="33.02" width="0.1524" layer="91"/>
<label x="109.22" y="33.02" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="KEY_BAK" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="1_BAK"/>
<wire x1="2.54" y1="149.86" x2="-2.54" y2="149.86" width="0.1524" layer="91"/>
<label x="-2.54" y="149.86" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D7"/>
<wire x1="48.26" y1="96.52" x2="43.18" y2="96.52" width="0.1524" layer="91"/>
<label x="43.18" y="96.52" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="KEY_CON" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="2_CON"/>
<wire x1="2.54" y1="147.32" x2="-2.54" y2="147.32" width="0.1524" layer="91"/>
<label x="-2.54" y="147.32" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D6"/>
<wire x1="48.26" y1="99.06" x2="43.18" y2="99.06" width="0.1524" layer="91"/>
<label x="43.18" y="99.06" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="N_F1_D1" class="1">
<segment>
<pinref part="D1" gate="G$1" pin="A"/>
<wire x1="203.2" y1="157.48" x2="198.12" y2="157.48" width="0.1524" layer="91"/>
<label x="198.12" y="157.48" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="F1" gate="G$1" pin="2"/>
<wire x1="269.24" y1="157.48" x2="274.32" y2="157.48" width="0.1524" layer="91"/>
<label x="274.32" y="157.48" size="1.27" layer="95"/>
</segment>
</net>
<net name="PWM_D9" class="0">
<segment>
<pinref part="R3" gate="G$1" pin="1"/>
<wire x1="187.96" y1="129.54" x2="182.88" y2="129.54" width="0.1524" layer="91"/>
<label x="182.88" y="129.54" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R4" gate="G$1" pin="1"/>
<wire x1="187.96" y1="114.3" x2="182.88" y2="114.3" width="0.1524" layer="91"/>
<label x="182.88" y="114.3" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D9"/>
<wire x1="48.26" y1="91.44" x2="43.18" y2="91.44" width="0.1524" layer="91"/>
<label x="43.18" y="91.44" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="SCL_3V3" class="0">
<segment>
<pinref part="J4" gate="G$1" pin="2_SCL"/>
<wire x1="210.82" y1="33.02" x2="205.74" y2="33.02" width="0.1524" layer="91"/>
<label x="205.74" y="33.02" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R2" gate="G$1" pin="2"/>
<wire x1="162.56" y1="30.48" x2="167.64" y2="30.48" width="0.1524" layer="91"/>
<label x="167.64" y="30.48" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="LV2"/>
<wire x1="200.66" y1="78.74" x2="205.74" y2="78.74" width="0.1524" layer="91"/>
<label x="205.74" y="78.74" size="1.27" layer="95"/>
</segment>
</net>
<net name="SCL_5V" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="A5/SCL"/>
<wire x1="73.66" y1="99.06" x2="78.74" y2="99.06" width="0.1524" layer="91"/>
<label x="78.74" y="99.06" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="HV2"/>
<wire x1="180.34" y1="78.74" x2="175.26" y2="78.74" width="0.1524" layer="91"/>
<label x="175.26" y="78.74" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="SDA_3V3" class="0">
<segment>
<pinref part="J4" gate="G$1" pin="1_SDA"/>
<wire x1="210.82" y1="35.56" x2="205.74" y2="35.56" width="0.1524" layer="91"/>
<label x="205.74" y="35.56" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R1" gate="G$1" pin="2"/>
<wire x1="162.56" y1="45.72" x2="167.64" y2="45.72" width="0.1524" layer="91"/>
<label x="167.64" y="45.72" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="LV1"/>
<wire x1="200.66" y1="81.28" x2="205.74" y2="81.28" width="0.1524" layer="91"/>
<label x="205.74" y="81.28" size="1.27" layer="95"/>
</segment>
</net>
<net name="SDA_5V" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="A4/SDA"/>
<wire x1="73.66" y1="101.6" x2="78.74" y2="101.6" width="0.1524" layer="91"/>
<label x="78.74" y="101.6" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="U4" gate="G$1" pin="HV1"/>
<wire x1="180.34" y1="81.28" x2="175.26" y2="81.28" width="0.1524" layer="91"/>
<label x="175.26" y="81.28" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="TACH_5V" class="0">
<segment>
<pinref part="C5" gate="G$1" pin="1"/>
<wire x1="152.4" y1="76.2" x2="147.32" y2="76.2" width="0.1524" layer="91"/>
<label x="147.32" y="76.2" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="D3" gate="G$1" pin="A"/>
<wire x1="152.4" y1="106.68" x2="147.32" y2="106.68" width="0.1524" layer="91"/>
<label x="147.32" y="106.68" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="D4" gate="G$1" pin="K"/>
<wire x1="167.64" y1="91.44" x2="172.72" y2="91.44" width="0.1524" layer="91"/>
<label x="172.72" y="91.44" size="1.27" layer="95"/>
</segment>
<segment>
<pinref part="R6" gate="G$1" pin="1"/>
<wire x1="187.96" y1="99.06" x2="182.88" y2="99.06" width="0.1524" layer="91"/>
<label x="182.88" y="99.06" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R7" gate="G$1" pin="1"/>
<wire x1="218.44" y1="91.44" x2="213.36" y2="91.44" width="0.1524" layer="91"/>
<label x="213.36" y="91.44" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="U1" gate="G$1" pin="D2"/>
<wire x1="48.26" y1="109.22" x2="43.18" y2="109.22" width="0.1524" layer="91"/>
<label x="43.18" y="109.22" size="1.27" layer="95" rot="R180"/>
</segment>
</net>
<net name="TACH_IN" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="2_TACH"/>
<wire x1="279.4" y1="124.46" x2="274.32" y2="124.46" width="0.1524" layer="91"/>
<label x="274.32" y="124.46" size="1.27" layer="95" rot="R180"/>
</segment>
<segment>
<pinref part="R7" gate="G$1" pin="2"/>
<wire x1="233.68" y1="91.44" x2="238.76" y2="91.44" width="0.1524" layer="91"/>
<label x="238.76" y="91.44" size="1.27" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
