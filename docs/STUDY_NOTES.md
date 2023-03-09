# PBR

##### Radiant Flux

Radiant energy per time： $\phi=\frac{dQ}{dt}$ (unit: [W] or [J/S])

##### Radiant Intensity

Radiant flux per solid angle: $I = \frac{d\phi}{d\omega}$

e.g. Point Light: $I = \frac{\phi_{pointlight}}{4\pi}$

##### Irradiance

Radiant flux per area: $E = \frac{d\phi}{dA}$ (unit: [$W/m^2$]) 

e.g. A point light and distance $r$, the angle of incidence is $\theta$, we can calculate the irradiance:

$E = \frac{d\phi}{dA} = \frac{I d\omega}{dA}$

$d\omega = \frac{dS}{r^2}$

$S$ is the area $\perp$ light direction, so $dS = cos(\theta) dA$, finally, we get

$E = \frac{I\cos(\theta)}{r^2} = \frac{cos(\theta)\phi_{pointlight}}{4\pi r^2}$

* irradiance decreases quadratically with distance.
* irradiance decreases as the angle $\theta$ increases

##### Radiance

**Radiant flux** per **solid angle** per **projected area**: $L = \frac{d^2\phi}{d\omega\ cos(\theta)dA}$

**IMPORTANT: Does not change with viewing distance**.

$L = \frac{dE}{d\omega cos(\theta)}$ 

so $dE = L\ cos(\theta)\ d\omega$

$E = \int_{\Omega_{h}}L\cos(\theta)\ d\omega$

##### Rendering Equation

$$
L_{o}(v) = L_{e}(v)\ +\ \int_{\Omega_{h}}f_{r}(v,l)\ L_{i}\ cos(\theta)d\omega
$$

$L_o$: outgoing radiance

$L_i$: incoming radiance

$f_r$: BRDF $f_r(v,l)=\frac{dL_o(v)}{dE(l)}$

##### 

##### Phong BRDF

$$
f_{phong}(l,v,n)=k_{ambient} + k_{diffuse}(n\cdot l) + k_{specular}(r \cdot v)^{\alpha}
$$

* $\alpha$:  the shininess or roughness exponent, which determines the width of the specular lobe
