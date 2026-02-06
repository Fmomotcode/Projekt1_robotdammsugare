        const slider = document.querySelector(".slider");
        const handle = document.querySelector(".slider-handle");
        const text = document.querySelector(".slider-text");
        
        let isDragging = false;
        let startX;
        let maxLeft = slider.offsetWidth - handle.offsetWidth - 2;
        let isOn = false;
        
        handle.addEventListener("touchstart", (e) => {
            isDragging = true;
            startX =  e.touches[0].clientX - handle.offsetLeft;
            handle.style.cursor = "grabbing";
        });
        
        document.addEventListener("touchmove", (e) => {
            if (!isDragging) return;
        
            let newLeft = e.touches[0].clientX - startX;
            if (newLeft < 2) newLeft = 2;
            if (newLeft > maxLeft) newLeft = maxLeft;
        
            handle.style.left = newLeft + "px";
        });
        
        document.addEventListener("touchend", () => {
            if (!isDragging) return;
            isDragging = false;
            handle.style.cursor = "grab";
        
            if (handle.offsetLeft > maxLeft / 2) {
                handle.style.left = maxLeft + "px";
                slider.style.backgroundColor = "#7b8c78";
                text.textContent = "ON";
                isOn = true;
        
                fetch('http://172.17.2.73:5000/start', { method: 'POST' })
                .then(response => console.log("Start response:", response.status))
                .catch(err => console.error("Fel vid start:", err));

                //fetch('http://192.168.0.59:5000/start', { method: 'POST' })
                //.then(response => console.log("Start response:", response.status))
                //.catch(err => console.error("Fel vid start:", err));
        
            } else {
                handle.style.left = "3px";
                slider.style.backgroundColor = "beige";
                text.textContent = "OFF";
                isOn = false;
        
                //fetch('http://172.17.2.73:5000/stop', { method: 'POST' })
                //.then(response => console.log("Stop response:", response.status))
                //.catch(err => console.error("Fel vid stop:", err));

                fetch('http://172.17.2.74:5000/stop', { method: 'POST' })
                .then(response => console.log("Stop response:", response.status))
                .catch(err => console.error("Fel vid stop:", err));
            }
        });
