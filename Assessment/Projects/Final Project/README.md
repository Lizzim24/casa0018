# Final Project – Smart Pot Lid Timer

<img src="../../Report/images/smart_pot_lid_timer.png" width="500">

*Figure 1. Final hardware prototype of the Smart Pot Lid Timer.*

## Definition of Problem Being Solved

This project investigates how embedded machine learning can enable reliable contactless interaction in noisy kitchen environments. Traditional interaction methods such as touchscreens and voice assistants are often unsuitable during cooking due to hygiene concerns and environmental noise.

The project proposes an alternative approach by transforming a pot lid into an intelligent interaction interface using embedded motion sensing. Natural gestures such as placing, rotating, and lifting the lid are recognised using IMU sensor data collected from the Arduino Nano 33 BLE Sense.

The system was designed as a real-time embedded pipeline consisting of:

* IMU-based sensing
* Edge Impulse gesture classification
* TensorFlow Lite for Microcontrollers deployment
* FSM-based timer control
* LED and buzzer feedback

The primary research question explored was:

> How can embedded sensing and machine learning support robust contactless interaction in noisy kitchen environments?

The anticipated outcome was a lightweight, real-time TinyML system capable of recognising natural kitchen gestures while operating fully on-device.

<img src="../../Report/images/pipeline.png" width="850">

*Figure 2. Edge Impulse pipeline for embedded gesture recognition.*

---

## Documentation of Experiments and Results

Multiple experiments were conducted to improve model performance and reduce overfitting. Early training runs used variable-length sampling windows and larger model architectures, which resulted in strong validation accuracy but poor generalisation on unseen test data.

Several refinements were introduced during development:

* Transition from variable windows to fixed 1000 ms windows
* Increased user diversity during data collection
* Low-pass filtering to reduce environmental vibration noise
* Reduced neural network complexity
* Confidence-threshold filtering during deployment

The final model used:

* 252 input features
* Two dense layers (20 and 10 neurons)
* 50 training epochs
* Learning rate of 0.0005

Final results:

* Validation accuracy: 92.5%
* Test accuracy: 81.97%
* Inference latency: ~1 ms
* Peak RAM usage: 1.6 KB

The project demonstrated that lightweight embedded gesture recognition is feasible on resource-constrained hardware.

<img src="../../Report/images/model_architecture.png" width="700">

*Figure 3. Lightweight neural network architecture used for deployment.*

<img src="../../Report/images/Confusion-train.png" width="650">

*Figure 4. Confusion matrix for held-out train dataset.*

<img src="../../Report/images/Confusion-test.png" width="650">

*Figure 5. Confusion matrix for held-out test dataset.*


---

## Critical Reflection and Learning from Experiments

<img src="../../Report/images/states.png" width="700">

*Figure 6. Different interaction states including countdown, warning, and alarm modes.*

One of the key lessons from this project was that data quality and preprocessing had a greater impact on performance than increasing model complexity. Early datasets contained inconsistent motion trajectories, which caused overfitting despite high validation accuracy.

Environmental noise also proved to be a major challenge. Vibrations caused by boiling water and cooking activity occasionally triggered false positives. Implementing a 10 Hz low-pass filter significantly improved robustness by removing high-frequency noise while preserving intentional gestures.

Another limitation was the reliance on a single lid during training. Although multiple users improved gesture diversity, the model may still partially depend on hardware-specific properties such as lid weight and inertia.

Future improvements could include:

* Larger multi-user datasets
* Different lid materials and sizes
* Hybrid ML + rule-based rotation detection
* Adaptive confidence thresholds
* Improved enclosure design using 3D printing

Overall, the project demonstrates the importance of balancing model performance, embedded constraints, and real-world usability in edge AI systems.
