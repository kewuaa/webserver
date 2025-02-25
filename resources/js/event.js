const CHUNK_SIZE = 1024 * 1024


let download_buttons = document.getElementsByClassName("download_button");
for (let i = 0; i < download_buttons.length; i++) {
    let button = download_buttons[i];
    button.addEventListener(
        "click",
        function(event) {
            const link = document.createElement("a")
            link.href = "/download?file="+event.target.value
            link.download = event.target.value.split("/").pop()

            document.body.append(link)
            link.click()

            document.body.removeChild(link)
        }
    )
}


document
    .getElementById("upload_button")
    .addEventListener(
        "click",
        async function(event) {
            const file_input = document.getElementById("file_input")
            const file = file_input.files[0]
            if (!file) {
                alert("choose a file first")
                return
            }
            const cwd = event.target.value;
            for (let start = 0; start < file.size; start += CHUNK_SIZE) {
                const response = await fetch(
                    "/upload",
                    {
                        method: "POST",
                        headers: {
                            "Content-Type": "application/octet-stream",
                            "X-file-name": encodeURI(file.name),
                            "X-cwd": encodeURI(cwd)
                        },
                        body: file.slice(start, start+CHUNK_SIZE)
                    }
                )
                if (!response.ok) {
                    throw new Error(`HTTP Error! status code: ${response.status}, status message: ${response.statusText}`)
                }
                const resp = await response.json()
                if (resp["status"] != 0) {
                    alert(resp["error_msg"])
                    return
                }
            }
            const response = await fetch(
                "/upload-done",
                {
                    method: "POST",
                    headers: {
                        "X-file-name": encodeURI(file.name),
                        "X-cwd": encodeURI(cwd)
                    },
                }
            )
            if (!response.ok) {
                throw new Error(`HTTP Error! statuc code: ${response.status}, status message: ${response.statusText}`)
            }
            alert("successfully upload")
            window.location.reload()
        }
    )
