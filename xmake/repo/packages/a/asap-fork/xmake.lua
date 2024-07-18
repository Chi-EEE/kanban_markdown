package("asap-fork")
    set_base("asap")
    
    add_patches("2023.04.21", path.join(os.scriptdir(), "patches", "2023.04.21", "public.patch"), "82c4b4d7a6c92ae22f176831ab7307fcf64553cc1e8fd3add9f4a24b2664ada3")
