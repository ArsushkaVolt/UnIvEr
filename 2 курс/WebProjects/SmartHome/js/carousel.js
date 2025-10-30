document.addEventListener('DOMContentLoaded', () => {
    const track = document.querySelector('.carousel-track');
    const items = document.querySelectorAll('.carousel-item');
    const prevButton = document.querySelector('.carousel-prev');
    const nextButton = document.querySelector('.carousel-next');
    const carouselContainer = document.querySelector('.review-carousel'); // Для hover
    if (!track || !items.length || !carouselContainer) return; // Проверка на наличие

    let currentIndex = 0;
    const totalItems = items.length;
    let intervalId = null;
    let isPaused = false;

    // Debounce функция
    function debounce(func, delay) {
        let timeout;
        return (...args) => {
            clearTimeout(timeout);
            timeout = setTimeout(() => func(...args), delay);
        };
    }

    function getItemsPerView() {
        return window.innerWidth <= 768 ? 1 : 3;
    }

    function updateCarousel() {
        const itemsPerView = getItemsPerView();
        const maxIndex = totalItems - itemsPerView;
        if (currentIndex > maxIndex) currentIndex = maxIndex; // Корректировка индекса
        const itemWidth = 100 / itemsPerView;
        requestAnimationFrame(() => {
            track.style.transform = `translateX(-${currentIndex * itemWidth}%)`;
        });

        // Отключить кнопки если не нужно
        prevButton.disabled = currentIndex === 0;
        nextButton.disabled = currentIndex >= maxIndex;

        // Отключить авто если все видимы
        if (itemsPerView >= totalItems) {
            clearInterval(intervalId);
            intervalId = null;
        } else if (!intervalId && !isPaused) {
            startAutoSlide();
        }
    }

    function nextSlide() {
        const itemsPerView = getItemsPerView();
        const maxIndex = totalItems - itemsPerView;
        currentIndex = (currentIndex + 1 > maxIndex) ? 0 : currentIndex + 1;
        updateCarousel();
    }

    function prevSlide() {
        const itemsPerView = getItemsPerView();
        currentIndex = (currentIndex - 1 < 0) ? (totalItems - itemsPerView) : currentIndex - 1;
        updateCarousel();
    }

    function startAutoSlide() {
        if (intervalId) clearInterval(intervalId);
        intervalId = setInterval(nextSlide, 5000);
    }

    // События
    nextButton.addEventListener('click', nextSlide);
    prevButton.addEventListener('click', prevSlide);

    // Пауза на hover
    carouselContainer.addEventListener('mouseenter', () => {
        isPaused = true;
        clearInterval(intervalId);
    });
    carouselContainer.addEventListener('mouseleave', () => {
        isPaused = false;
        if (getItemsPerView() < totalItems) {
            startAutoSlide();
        }
    });

    // Debounced resize
    const debouncedUpdate = debounce(updateCarousel, 200);
    window.addEventListener('resize', debouncedUpdate);

    // Инициализация
    updateCarousel();
    if (getItemsPerView() < totalItems) {
        startAutoSlide();
    }
});