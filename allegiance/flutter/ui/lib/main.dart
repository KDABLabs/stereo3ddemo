import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runApp(const AllegianceApp());
}

class AllegianceApp extends StatelessWidget {
  const AllegianceApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const Allegiance(title: 'Allegiance'),
    );
  }
}

class Allegiance extends StatefulWidget {
  const Allegiance({super.key, required this.title});

  final String title;

  @override
  State<Allegiance> createState() => _AllegianceState();
}

class _AllegianceState extends State<Allegiance> {
  int _counter = 0;

  void _setDisplayMode(int mode) async {
    final WriteBuffer buffer = WriteBuffer()..putInt64(mode);
    final ByteData message = buffer.done();
    await ServicesBinding.instance.defaultBinaryMessenger
        .send('allegiance/set-mode', message);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.transparent,
      appBar: AppBar(title: const Text('Allegiance')),
      body: Align(
        alignment: Alignment.bottomRight,
        child: Container(
          margin: const EdgeInsets.all(10.0),
          decoration: BoxDecoration(
              color: Colors.white,
              borderRadius: BorderRadius.all(Radius.circular(20))),
          child: OverflowBar(
            alignment: MainAxisAlignment.spaceEvenly,
            children: <Widget>[
              TextButton(
                  child: const Text('Image'),
                  onPressed: () {
                    _setDisplayMode(0);
                  }),
              TextButton(
                  child: const Text('3D Scene'),
                  onPressed: () {
                    _setDisplayMode(1);
                  }),
            ],
          ),
        ),
      ),
    );
  }
}
