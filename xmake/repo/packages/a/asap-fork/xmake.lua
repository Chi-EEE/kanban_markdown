package("asap-fork")
    set_base("asap")
    
    add_patches("2023.04.21", path.join(os.scriptdir(), "patches", "2023.04.21", "public.patch"), "c4624bb82239c680bcc72b20eeb99a010aef1c4349d4133e3a2979b485f2c4c7")
