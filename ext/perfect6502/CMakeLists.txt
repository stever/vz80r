fips_begin_lib(perfectz80)
    fips_vs_warning_level(3)
    fips_files(
        netlist_z80.h types.h
        netlist_sim.h netlist_sim.c
        perfectz80.h perfectz80.c
    )
fips_end_lib()
target_include_directories(perfectz80 INTERFACE .)
