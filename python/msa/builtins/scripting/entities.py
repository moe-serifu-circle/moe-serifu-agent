import datetime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import (
    Column, Integer, MetaData, Table, Text, DateTime, create_engine, select, ForeignKey, String, inspect)
from sqlalchemy.schema import CreateTable, DropTable

Base = declarative_base()


async def setup(db):
    Base.metadata.create_all(engine)




class ScriptEntity(Base):
    __tablename__ = 'scripts'
    
    id = Column(Integer, primary_key=True)
    name = Column(String)
    crontab = Column(String)
    created = Column(DateTime, default=datetime.datetime.now)
    last_edited = Column(DateTime, default=datetime.datetime.now)
    last_run = Column(DateTime, default=datetime.datetime.now)
    script_contents = Column(String)

class ScriptRunResultEntity(Base):
    __tablename__ = "script_run_result"
    
    id = Column(Integer, primary_key=True)
    script = Column(ForeignKey('scripts.id'))
    run_log = Column(String)
    run_result = Column(String)
    created = Column(DateTime, default=datetime.datetime.now)


