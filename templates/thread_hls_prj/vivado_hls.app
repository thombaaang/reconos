<<reconos_preproc>><?xml version="1.0" encoding="UTF-8"?>
<project xmlns="com.autoesl.autopilot.project" top="rt_imp">
  <files>
      <file name="<<HLSDIR>>/reconos_calls.h" sc="0" tb="false" cflags="-D__SYNTHESIS__"/>
      <file name="<<HLSDIR>>/reconos_thread.h" sc="0" tb="false" cflags="-D__SYNTHESIS__"/>
    <<generate for FILES>>
      <file name="<<HLSDIR>>/<<File>>" sc="0" tb="false" cflags="-D__SYNTHESIS__"/>
    <<end generate>>
  </files>
  <solutions>
    <solution name="sol" status="active"/>
  </solutions>
  <includePaths/>
  <libraryPaths/>
  <Simulation>
    <SimFlow name="csim"/>
  </Simulation>
</project>
