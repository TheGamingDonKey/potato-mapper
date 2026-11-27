const watchLink = document.querySelector(".watch-preview");
const preview = document.querySelector("#motion-preview");
const video = preview.querySelector("video");
let previewSource;

function loadPreview() {
  if (!previewSource) {
    // A local Blob lets the player seek on hosts that return the whole MP4 for byte ranges.
    previewSource = fetch(watchLink.href)
      .then(response => {
        if (!response.ok) throw new Error("The video could not be loaded.");
        return response.blob();
      })
      .then(blob => URL.createObjectURL(blob))
      .catch(() => {
        previewSource = undefined;
        return watchLink.href;
      });
  }
  return previewSource;
}

if (typeof preview.showModal === "function") {
  watchLink.setAttribute("aria-haspopup", "dialog");
  watchLink.addEventListener("click", async event => {
    if (event.ctrlKey || event.metaKey || event.shiftKey || event.altKey || event.button !== 0) return;
    event.preventDefault();
    preview.showModal();
    const source = await loadPreview();
    if (!preview.open) return;
    if (video.getAttribute("src") !== source) video.src = source;
    video.play().catch(() => {
      // Browsers that require a separate play gesture still show the native controls.
    });
  });
  preview.addEventListener("close", () => video.pause());
}
