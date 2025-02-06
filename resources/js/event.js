document
    .getElementById("download_button")
    .addEventListener(
        "click",
        function(event) {
            const link = document.createElement("a")
            link.href = "/download?file="+event.target.value
            link.download = event.target.value.split("/").pop()

            document.body.append(link)
            link.click()

            document.removeChild(link)
        }
    )
