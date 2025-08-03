<?php
header("Content-Type: text/html");
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Infinite 3D Fractal Animation</title>
    <style>
        body { margin: 0; overflow: hidden; background: black; }
        canvas { display: block; }
    </style>
</head>
<body>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/simplex-noise/2.4.0/simplex-noise.min.js"></script>
    <script>
        // Setup scene, camera, renderer
        const scene = new THREE.Scene();
        const randomX = (Math.random() - 0.9) * 10; // Between -5 and 5
        const randomY = Math.random() * 9 + 1; // Between 1 and 6
        const randomZ = (Math.random() - 0.9) * 10; // Between -5 and 5

        const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
        camera.position.set(randomX, randomY, randomZ);
        camera.lookAt(0, 0, 0);
        const renderer = new THREE.WebGLRenderer({ antialias: true });
        renderer.setSize(window.innerWidth, window.innerHeight);
        document.body.appendChild(renderer.domElement);

        // Lighting
        const light = new THREE.DirectionalLight(0xffffff, 1);
        light.position.set(5, 5, 5).normalize();
        scene.add(light);

        // Infinite fractal parameters
        const size = 4, segments = 64;
        const geometry = new THREE.PlaneGeometry(size, size, segments, segments);
        const material = new THREE.MeshStandardMaterial({ color: 0x00ff00, flatShading: true });

        const noise = new SimplexNoise();
        const positions = geometry.attributes.position.array;

        function updateFractal(time) {
            for (let i = 0; i < positions.length; i += 3) {
                const x = positions[i], y = positions[i + 1];
                positions[i + 2] = noise.noise2D(x * 2, y * 2 + time * 0.1) * 0.5;
            }
            geometry.attributes.position.needsUpdate = true;
        }

        const fractal = new THREE.Mesh(geometry, material);
        fractal.rotation.x = -Math.PI / 2;
        scene.add(fractal);

        camera.position.set(0, 3, 5);
        camera.lookAt(scene.position);

        // Infinite animation loop
        function animate(time) {
            requestAnimationFrame(animate);
            updateFractal(time * 0.001); // Smooth scrolling effect
            fractal.rotation.y += 0.009; // Slow rotation
            fractal.rotation.x += 0.009; // Slow rotation
            fractal.rotation.z += 0.009; // Slow rotation
            renderer.render(scene, camera);
        }
        animate();

        // Resize handling
        window.addEventListener('resize', () => {
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();
            renderer.setSize(window.innerWidth, window.innerHeight);
        });
    </script>
</body>
</html>
