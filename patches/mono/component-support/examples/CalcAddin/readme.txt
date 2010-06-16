unfortunately no time to create some proper makefiles etc. However you can easily build the simple component manually as follows

prequisites:
 o a mono enabled openoffice build
 o a build with the the mono component/extension loader support ;-) ( see ../../mono-component-support.diff )

to test 

1. set up path to needed assemblies

  export CLI_URE=$(office_install_path)/ure/lib

2. compile the example component 
  a) best to build the java version from the odk first ( or you can extract the provided one from the prepared oxt file here ) in order to get the rdb file
    e.g. unzip  MonoCalcAddins.oxt CalcAddins.uno.rdb
  b) use climaker on the rdb to create cli_CalcAddins.dll.
     e.g in a new shell, in your build tree
     cd cli_ure
     export MONO_PATH=./unxlngx6.pro/bin/
     export LD_LIBRARY_PATH=../solver/320/unxlngx6.pro/lib/
     ./unxlngx6.pro/bin/climaker --out cli_CalcAddins.dll --reference ./unxlngx6.pro/bin/cli_uretypes.dll --extra ../solver/320/unxlngx6.pro/bin/types.rdb CalcAddins.uno.rdb
  c) mcs -target:library CalcAddins.cs -r:$CLI_URE/cli_basetypes.dll      -r:$CLI_URE/cli_uretypes.dll -r:$CLI_URE/cli_cppuhelper.dll -r:$CLI_URE/cli_ure.dll -r:$CLI_URE/cli_uno_bridge.dll -r:$CLI_URE/../../basis-link/program/cli_oootypes.dll -r:cli_CalcAddins.dll

3. insert into prepared extension bundle ( will overwrite existing content )

  zip MonoCalcAddins.oxt CalcAddins.dll cli_CalcAddins.dll CalcAddins.uno.rdb

4. to deploy:

  you need to prepare ( as of OOO3.2 ( OOO300_m19 ) ) your installation

  a). in the  $(office_install_path)/ure/lib 
   link libsal.so -> libuno_sal.so.3
   link libuno_cppu.so -> libuno_cppu.so.3

     e.g. ln -s libuno_sal.so.3 libsal.so 
      ln -s libuno_cppu.so.3 libuno_cppu.so

  b) set up your LD_LIBRARY_PATH ( this sucks )
    export LD_LIBRARY_PATH=$(office_install_path)/ure/lib:$ LD_LIBRARY_PATH
  c) set up your MONO_PATH
    export MONO_PATH=$(office_install_path)/ure/lib:$(office_install_path)/basis-link/program

  d) launch the office ( from the shell with env variables as above )
  e) use the extension manager to install the extension ( e.g Tools | ExtensionManager )
  f) restart office
  g) open CalcAddins.ods ( from odk/examples/java/Spreadsheet ) and read the text, you should be able to see the addin functions returning numbers ( if it worked )
