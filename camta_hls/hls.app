<AutoPilot:project xmlns:AutoPilot="com.autoesl.autopilot.project" top="camta" name="camta_hls">
    <files>
        <file name="../../tb/camta_tb.cpp" sc="0" tb="1" cflags="-Wno-unknown-pragmas" csimflags="" blackbox="false"/>
        <file name="src/horner_core.h" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
        <file name="src/camta.h" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
        <file name="src/horner_core.cpp" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
        <file name="src/camta.cpp" sc="0" tb="false" cflags="" csimflags="" blackbox="false"/>
    </files>
    <solutions>
        <solution name="solution1" status=""/>
    </solutions>
    <Simulation argv="">
        <SimFlow name="csim" setup="false" optimizeCompile="false" clean="true" ldflags="" mflags=""/>
    </Simulation>
</AutoPilot:project>

