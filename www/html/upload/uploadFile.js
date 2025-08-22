const fileUpload = document.getElementById('file-upload');
const fileBrowser = document.getElementById('file-browser');
const borderContainer = document.querySelector('.border-container');

// Open file browser when clicking "browse"
fileBrowser.addEventListener('click', (e) => {
  e.preventDefault();
  fileUpload.click();
});

// Handle file input
fileUpload.addEventListener('change', () => {
  alert(`Selected file: ${fileUpload.files[0].name}`);
});

// Drag & drop highlight
borderContainer.addEventListener('dragover', (e) => {
  e.preventDefault();
  borderContainer.classList.add('dragover');
});

borderContainer.addEventListener('dragleave', () => {
  borderContainer.classList.remove('dragover');
});

borderContainer.addEventListener('drop', (e) => {
  e.preventDefault();
  borderContainer.classList.remove('dragover');
  if (e.dataTransfer.files.length > 0) {
    alert(`Dropped file: ${e.dataTransfer.files[0].name}`);
  }
});
