document.addEventListener('DOMContentLoaded', () => {

    // Terminal Typewriter Effect
    const terminalText = [
        "Booting from Hard Disk...",
        "RudzaniOS Bootloader v1.0",
        "Loading kernel into memory...",
        "[OK] Entering 32-bit protected mode.",
        "[OK] PMM Array initialized.",
        "[OK] Mounting root VFS (/)",
        "RudzaniOS:~# lets_play"
    ];

    const typeWriterEl = document.getElementById('typewriter-demo');
    let lineIdx = 0;
    
    function typeLine(text, index) {
        if (index < text.length) {
            typeWriterEl.innerHTML += text.charAt(index);
            setTimeout(() => typeLine(text, index + 1), Math.random() * 50 + 20);
        } else {
            typeWriterEl.innerHTML += '<br>';
            lineIdx++;
            if (lineIdx < terminalText.length) {
                setTimeout(startTyping, 500);
            } else {
                // Add blinking cursor
                typeWriterEl.innerHTML += '<span class="cursor">_</span>';
                addCursorAnimation();
            }
        }
    }

    function startTyping() {
        if(lineIdx < terminalText.length) {
            typeLine(terminalText[lineIdx], 0);
        }
    }

    function addCursorAnimation() {
        const style = document.createElement('style');
        style.innerHTML = `
            @keyframes blink {
                0%, 100% { opacity: 1; }
                50% { opacity: 0; }
            }
            .cursor { animation: blink 1s step-end infinite; }
        `;
        document.head.appendChild(style);
    }

    setTimeout(startTyping, 1000);


    // Copy Hash Functionality
    const copyButtons = document.querySelectorAll('.btn-copy');
    copyButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const targetId = btn.getAttribute('data-target');
            const hashText = document.getElementById(targetId).textContent;
            
            navigator.clipboard.writeText(hashText).then(() => {
                const icon = btn.querySelector('i');
                const originalIcon = icon.getAttribute('data-lucide');
                
                // Show checkmark
                icon.setAttribute('data-lucide', 'check');
                btn.style.color = '#00ffcc';
                lucide.createIcons();
                
                // Reset after 2 seconds
                setTimeout(() => {
                    icon.setAttribute('data-lucide', originalIcon);
                    btn.style.color = '';
                    lucide.createIcons();
                }, 2000);
            });
        });
    });

    // Particles JS Configuration
    if(window.particlesJS) {
        particlesJS("particles-js", {
            "particles": {
                "number": { "value": 40, "density": { "enable": true, "value_area": 800 } },
                "color": { "value": "#00ffcc" },
                "shape": { "type": "circle" },
                "opacity": { "value": 0.3, "random": false },
                "size": { "value": 3, "random": true },
                "line_linked": {
                    "enable": true,
                    "distance": 150,
                    "color": "#00ffcc",
                    "opacity": 0.2,
                    "width": 1
                },
                "move": {
                    "enable": true,
                    "speed": 2,
                    "direction": "none",
                    "random": false,
                    "straight": false,
                    "out_mode": "out",
                    "bounce": false,
                }
            },
            "interactivity": {
                "detect_on": "canvas",
                "events": {
                    "onhover": { "enable": true, "mode": "grab" },
                    "onclick": { "enable": true, "mode": "push" },
                    "resize": true
                },
                "modes": {
                    "grab": { "distance": 140, "line_linked": { "opacity": 1 } },
                    "push": { "particles_nb": 4 }
                }
            },
            "retina_detect": true
        });
    }

});
