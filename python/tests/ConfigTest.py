import config

cfg = load("../msa.cfg")
cfg.sections["agent"].push("user-title","Onii-chan") #testing multiple values
save(cfg, "
