document.addEventListener('DOMContentLoaded', () => {
    const track = document.querySelector('.carousel-track');
    const items = document.querySelectorAll('.carousel-item');
    const prevButton = document.querySelector('.carousel-prev');
    const nextButton = document.querySelector('.carousel-next');
    let currentIndex = 0;
    const totalItems = items.length;

    function updateCarousel() {
        const itemsPerView = window.innerWidth <= 768 ? 1 : 3;
        const itemWidth = 100 / itemsPerView;
        track.style.transform = `translateX(-${currentIndex * itemWidth}%)`;
    }

    function nextSlide() {
        const itemsPerView = window.innerWidth <= 768 ? 1 : 3;
        currentIndex = (currentIndex + 1) % (totalItems - itemsPerView + 1);
        updateCarousel();
    }

    function prevSlide() {
        const itemsPerView = window.innerWidth <= 768 ? 1 : 3;
        currentIndex = (currentIndex - 1 + (totalItems - itemsPerView + 1)) % (totalItems - itemsPerView + 1);
        updateCarousel();
    }

    nextButton.addEventListener('click', nextSlide);
    prevButton.addEventListener('click', prevSlide);
    window.addEventListener('resize', updateCarousel);
    updateCarousel();

    setInterval(() => {
    nextSlide();
}, 5000);


});