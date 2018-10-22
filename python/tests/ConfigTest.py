from msa.config import *
#only used * for simple namespace usage within this file only
#don't use in other files if possible as a general rule

try:
    cfg = load("../msa.cfg")
    print("loaded default config file")
    cfg.sections["agent"].push("user_title","Onii-chan") #testing multiple values
    save(cfg, "test.cfg") #testing save function
    print("saved new config file, check file to confirm correct output")

    new_cfg = load("test.cfg") #testing loading with multiple values
    if len(new_cfg.sections["agent"].get_all("user_title")) == 2:
        print("Test passed with multiple value assignments")
except Exception as e:
    print(e)

input("Press enter to exit")
