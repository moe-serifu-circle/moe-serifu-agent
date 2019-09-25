import datetime

from tortoise.models import Model
from tortoise import fields



class ScriptEntity(Model):
    id = fields.IntField(pk=True)
    name = fields.TextField()
    crontab = fields.TextField()
    created = fields.DatetimeField(auto_now_add=True)
    last_edited = fields.DatetimeField(auto_now=True)
    last_run = fields.DatetimeField(auto_now_add=True)
    script_contents = fields.TextField()

    def __str__(self):
        return f"ScriptEntity<{self.id}:{self.name}>"

class ScriptRunResultEntity(Model):
    id = fields.IntField(pk=True)
    script = fields.ForeignKeyField("models.ScriptEntity", related_name="script_run_results")
    run_log = fields.TextField()
    run_result = fields.TextField()
    created = fields.DatetimeField(auto_now_add=True)


