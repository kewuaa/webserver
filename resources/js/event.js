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
