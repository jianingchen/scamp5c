User Command 3.40       APRON Palettes   $(APRON)\apal.exe       $(APRON)                      SCAMP-5c Compile Simulation   $(APRON)\s5c-make.exe    -sim "$(FilePath)"
   $(FileDir)                    SCAMP-5c Run Simulation   $(APRON)\asim.exe 3   "$(FileDir)\$(FileTitle)_sim.apron2" rend.soft null
   $(FileDir)                     SCAMP-5c Compile ICW   $(APRON)\s5c-make.exe    -icw "$(FilePath)"
   $(FileDir)                    SCAMP-5c Host ICW   $(APRON)\asim.exe 8   "$(FileDir)\$(FileTitle)_icw.host.apron2" rend.soft null
   $(FileDir)                     SCAMP-5c Clean   $(APRON)\s5c-make.exe    -clean "$(FilePath)"
   $(FileDir)                    APRON Compiler   $(APRON)\acomp.exef   src="$(FilePath)" inc="$(APRON)\include;$(APRON)\plugins;$(APRON)\models" irl=10 verbosity=IWEM mrl=10
   $(FileDir)                    APRON Simulator   $(APRON)\asim.exe/   "$(FileDir)\$(FileTitle).apron2" rend.soft null
   $(FileDir)                     APRON Palettes   $(APRON)\apal.exe       $(APRON)                                                                                   