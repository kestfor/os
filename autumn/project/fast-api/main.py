import logging
import os

import fastapi
import uvicorn

app = fastapi.FastAPI()


@app.get("/file")
async def auth_callback(filename: str):
    if os.path.exists(filename):
        return fastapi.responses.FileResponse(path=filename)
    else:
        return fastapi.responses.Response(status_code=404, content="File Not Found")


if __name__ == "__main__":
    uvicorn.run(app, host='0.0.0.0', port=80)
