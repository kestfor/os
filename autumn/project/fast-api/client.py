import asyncio
import time

from aiohttp import ClientSession


def create_big_file(size_mb=1000):
    size_bytes = size_mb * 1024 * 1024
    bucket_size = 1024 * 1024
    with open("file", "w") as f:
        for bucket in range(0, size_bytes, bucket_size):
            f.write("a" * bucket_size)


async def test(url):
    # create_big_file()

    proxies = {
        "http": "http://172.23.68.224:80",
    }

    headers = {
        "Connection": "close"
    }

    async with ClientSession(headers=headers) as session:
        async with session.get(url, proxy=proxies["http"]) as response:
            if response.status == 200:
                start = time.time()
                total_read = 0
                async for data in response.content.iter_chunked(4096):
                    total_read += len(data)
                print(f"got data in ({time.time() - start}s, total_size: {total_read} bytes)")
            else:
                print(response.status, response.reason)


async def main():
    test_url = "http://192.168.56.1:80/file?filename=img.jpg"
    tasks_num = 1
    tasks = [asyncio.create_task(test(test_url)) for _ in range(tasks_num)]
    await asyncio.gather(*tasks)


if __name__ == '__main__':
    asyncio.run(main())
