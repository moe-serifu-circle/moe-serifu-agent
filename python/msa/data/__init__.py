from tortoise import Tortoise

from tortoise.backends.base.config_generator import generate_config

__models__ = []


async def start_db_engine():
    await Tortoise.init(
        db_url='sqlite://./msa.db',
        modules={'models': ['msa.data']}
    )
    await Tortoise.generate_schemas(safe=True)


async def stop_db_engine(_):
    await Tortoise.close_connections()




