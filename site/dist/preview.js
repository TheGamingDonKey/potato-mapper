const watchLink = document.querySelector(".watch-preview");
const preview = document.querySelector("#motion-preview");
const video = preview.querySelector("video");

if (typeof preview.showModal === "function") {
  watchLink.setAttribute("aria-haspopup", "dialog");
  watchLink.addEventListener("click", event => {
    if (event.ctrlKey || event.metaKey || event.shiftKey || event.altKey || event.button !== 0) return;
    event.preventDefault();
    preview.showModal();
    video.play().catch(() => {
      // Browsers that require a separate play gesture still show the native controls.
    });
  });
  preview.addEventListener("close", () => video.pause());
}
