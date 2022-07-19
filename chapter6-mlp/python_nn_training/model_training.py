import time
import os

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from keras import backend, optimizers
from keras.callbacks import EarlyStopping, ModelCheckpoint
from keras.layers import Activation, Dense
from keras.models import Sequential, load_model

# Methods in this file are responsible for training the networks 
# that can be later used in C++ framework or in atmo_rendering.py script

def rmse(y_true, y_pred):
    ''' rmse '''
    return backend.sqrt(backend.mean(backend.square(y_pred - y_true), axis=-1))

def save_nn_model(filename, model):
    ''' save_nn_model '''
    model.save(filename)
    print("Saved \"%s\" model to disk" % filename)

def load_dataset(filename: str, directory: str, num_skiprows: int = 0):
    ''' load_dataset '''

    start = time.time()

    data = pd.read_csv(directory + filename, sep=' ', header=None, skiprows=num_skiprows)
    dataset = data.values

    end = time.time()
    print("File loading time: %.2fs" % (end - start))

    return dataset

def plot_history(history, title_rmse: str, title_loss: str, rmse_output_filename: str, loss_output_filename: str, show_plots=False):
    ''' plot_history '''

    # summarize history for RMSE
    plt.plot(history.history['rmse'])
    plt.plot(history.history['val_rmse'])
    plt.title(title_rmse)
    plt.ylabel('rmse')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    if show_plots: plt.show()
    plt.savefig(rmse_output_filename)
    plt.close()

    # summarize history for Loss
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title(title_loss)
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    if show_plots: plt.show()
    plt.savefig(loss_output_filename)
    plt.close()

def teach_nn_lut_single_planet(dataset, _epochs, _batch_size, _learning_rate, val_split=0.1, num_hidden_layers=10, num_neurons=128, enable_checkpoints=False, checkpoint_filepath=""):
    ''' teach_nn_lut_single_planet '''

    # prepare data
    start = time.time()
    
    input_data = dataset
    np.random.shuffle(input_data)

    data_x = input_data[:, :3]
    data_y = input_data[:, 5:]
    
    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    ## Define NN model
    model = Sequential()
    model.add(Dense(num_neurons, input_shape=(3,)))
    model.add(Activation('relu'))

    for i in range(num_hidden_layers-1):
        model.add(Dense(num_neurons))
        model.add(Activation('relu'))

    model.add(Dense(4, activation='linear'))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        #checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=False, period=10)
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=True)
        early_stop = EarlyStopping(monitor='val_loss', patience=50)
        callback_list = [early_stop, checkpoint]

    # fit the keras model on the dataset (CPU)
    history = model.fit(data_x, data_y, epochs=_epochs, batch_size=_batch_size, validation_split=val_split, shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

def teach_nn_lut_multiple_planets(dataset, _epochs, _batch_size, _learning_rate, val_split=0.33, num_hidden_layers=20, num_neurons=128, enable_checkpoints=False, checkpoint_filepath=""):
    ''' teach_nn_lut_multiple_planets '''

    # prepare data
    start = time.time()
    
    input_data = dataset
    np.random.shuffle(input_data)
    
    data_x = input_data[:, :5]
    data_y = input_data[:, 5:]
    
    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    ## Define NN model
    model = Sequential()
    model.add(Dense(num_neurons, input_shape=(5,)))
    model.add(Activation('relu'))

    for i in range(num_hidden_layers-1):
        model.add(Dense(num_neurons))
        model.add(Activation('relu'))

    model.add(Dense(4, activation='linear'))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=False, period=5)
        callback_list = [checkpoint]

    # fit the keras model on the dataset (CPU)
    history = model.fit(data_x, data_y, epochs=_epochs, batch_size=_batch_size, validation_split=val_split, shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

def teach_img_based(dataset, _epochs, _batch_size, _learning_rate, val_split=0.2, num_hidden_layers=10, num_neurons=128, enable_checkpoints=False, checkpoint_filepath=""):
    ''' teach_img_based '''

    # prepare data
    start = time.time()
    
    input_data = dataset
    np.random.shuffle(input_data)

    data_x = input_data[:, :5]
    data_y = input_data[:, 5:]

    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    ## Define NN model
    model = Sequential()
    model.add(Dense(num_neurons, input_shape=(5,)))
    model.add(Activation('relu'))

    for i in range(num_hidden_layers-1):
        model.add(Dense(num_neurons))
        model.add(Activation('relu'))

    model.add(Dense(3, activation='linear'))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=True)
        early_stop = EarlyStopping(monitor='val_loss', patience=50)
        callback_list = [early_stop, checkpoint]

    # fit the keras model on the dataset (CPU)
    history = model.fit(data_x, data_y, epochs=_epochs, batch_size=_batch_size, validation_split=val_split, shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

def teach_img_based_custom_test_set(dataset_train, dataset_test, _epochs, _batch_size, _learning_rate, num_hidden_layers=10, num_neurons=128, enable_checkpoints=False, checkpoint_filepath=""):
    ''' teach_img_based_custom_test_set '''

    # prepare data
    start = time.time()
    
    input_data_train = dataset_train
    np.random.shuffle(input_data_train)

    data_x_train = input_data_train[:, :5]
    data_y_train = input_data_train[:, 5:]

    data_x_test = dataset_test[:, :5]
    data_y_test = dataset_test[:, 5:]

    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    ## Define NN model
    model = Sequential()
    model.add(Dense(num_neurons, input_shape=(5,)))
    model.add(Activation('relu'))

    for i in range(num_hidden_layers-1):
        model.add(Dense(num_neurons))
        model.add(Activation('relu'))

    model.add(Dense(3, activation='linear'))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=True)
        early_stop = EarlyStopping(monitor='val_loss', patience=50)
        callback_list = [early_stop, checkpoint]

    # fit the keras model on the dataset (CPU)
    history = model.fit(data_x_train, data_y_train, validation_data=(data_x_test, data_y_test), epochs=_epochs, batch_size=_batch_size, shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

def transfer_learn_model_img_based(model, dataset, _epochs, _batch_size, _learning_rate, val_split=0.2, enable_checkpoints=False, checkpoint_filepath=""):
    ''' transfer_learn_model '''

    # prepare data
    start = time.time()
    
    input_data = dataset
    np.random.shuffle(input_data)

    data_x = input_data[:, :5]
    data_y = input_data[:, 5:]

    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=True)
        early_stop = EarlyStopping(monitor='val_loss', patience=50)
        callback_list = [early_stop, checkpoint]

    # fit the keras model on the dataset (CPU)
    print("Transfer learning")
    history = model.fit(data_x, data_y, epochs=_epochs, batch_size=_batch_size, validation_split=val_split, shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

def transfer_learn_model_img_based_custom_test_set(model, dataset_train, dataset_test, _epochs, _batch_size, _learning_rate, enable_checkpoints=False, checkpoint_filepath=""):
    ''' transfer_learn_model custom test set'''

    # prepare data
    start = time.time()
    
    input_data_train = dataset_train
    np.random.shuffle(input_data_train)

    data_x_train = input_data_train[:, :5]
    data_y_train = input_data_train[:, 5:]

    data_x_test = dataset_test[:, :5]
    data_y_test = dataset_test[:, 5:]

    end = time.time()
    print("Datasets preparation time: %.2fs" % (end - start))

    # compile the keras model
    opt = optimizers.Adam(learning_rate=_learning_rate)

    model.compile(optimizer=opt, loss="mse", metrics=[rmse])
    model.summary()

    callback_list = []
    if(enable_checkpoints):
        checkpoint = ModelCheckpoint(checkpoint_filepath, verbose=1, save_best_only=True)
        early_stop = EarlyStopping(monitor='val_loss', patience=50)
        callback_list = [early_stop, checkpoint]

    # fit the keras model on the dataset (CPU)
    print("Transfer learning")
    history = model.fit(data_x_train, data_y_train, epochs=_epochs, batch_size=_batch_size, validation_data=(data_x_test, data_y_test), shuffle=True, verbose=1, callbacks=callback_list)

    return model, history

## Main training functions

def train_nn_single_planet():
    ''' train_nn_single_planet '''

    print("Model training: Single planet")

    dataset = load_dataset("ss_lut_512_128_64_64_64_rm_earth.txt", '../output/', num_skiprows=1)

    model_filename = "nn_lut_512_128_64_64_64_rm_earth_3layers"
    #model, history = teach_nn_lut_single_planet(dataset, _epochs=100, _batch_size=64, _learning_rate=0.00001, val_split=0.1, num_hidden_layers=10, num_neurons=128, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    model, history = teach_nn_lut_single_planet(dataset, _epochs=3000, _batch_size=32, _learning_rate=0.0001, val_split=0.2, num_hidden_layers=3, num_neurons=128, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model)

    plot_history(history, "RMSE during training", "Loss - during training", "res/" + model_filename + "_rmse.png", "res/" + model_filename + "_loss.png")

def train_nn_multi_planets():
    ''' train_nn_multi_planets '''

    print("Model training: Multiple planets")

    # Train only on Mars, Earth and Venus
    dataset_earth = load_dataset("ss_lut_512_128_128_128_128_rm_earth.txt", '../output/', num_skiprows=1)
    dataset_venus = load_dataset("ss_lut_512_128_128_128_128_rm_venus.txt", '../output/', num_skiprows=1)
    dataset_mars  = load_dataset("ss_lut_512_128_128_128_128_rm_mars.txt",  '../output/', num_skiprows=1)
    dataset_planets = np.concatenate((dataset_earth, dataset_venus, dataset_mars))
    model_filename = "nn_lut_512_128_128_128_128_rm_multi_planets"

    # Train only on 6 imaginary planets to recreate Earth, Venus and Mars
    # dataset_im1 = load_dataset("ss_lut_512_128_128_128_128_rm_im1.txt", '../output/', num_skiprows=1)
    # dataset_im2 = load_dataset("ss_lut_512_128_128_128_128_rm_im2.txt", '../output/', num_skiprows=1)
    # dataset_im3 = load_dataset("ss_lut_512_128_128_128_128_rm_im3.txt", '../output/', num_skiprows=1)
    # dataset_im4 = load_dataset("ss_lut_512_128_128_128_128_rm_im4.txt", '../output/', num_skiprows=1)
    # dataset_im5 = load_dataset("ss_lut_512_128_128_128_128_rm_im5.txt", '../output/', num_skiprows=1)
    # dataset_im6 = load_dataset("ss_lut_512_128_128_128_128_rm_im6.txt", '../output/', num_skiprows=1)
    # dataset_planets = np.concatenate((dataset_im1, dataset_im2, dataset_im3, dataset_im4, dataset_im5, dataset_im6))
    # model_filename = "nn_lut_512_128_128_128_128_rm_multi_planets_im"

    model, history = teach_nn_lut_multiple_planets(dataset_planets, _epochs=100, _batch_size=128*2, _learning_rate=0.00001, val_split=0.2, num_hidden_layers=250, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model)

    plot_history(history, "RMSE during training", "Loss - during training", "res/" + model_filename + "_rmse.png", "res/" + model_filename + "_loss.png")

def train_nn_img_based():
    ''' train_nn_img_based '''

    print("Model training: Image based")

    #dataset = load_dataset("dataset_py_hdr_even.txt", "hdr_merge/")
    #dataset = load_dataset("dataset_py_hdr_missing_no_02.txt", "hdr_merge/")
    #model_filename = "nn_img_based_real_256_256_10l_even"
    #model_filename = "nn_img_based_real_256_256_10l_missing_no_02"
    dataset = load_dataset("synth_ea_dataset.txt", "../output/figures/animation/")
    model_filename = "nn_img_based_synth_256_256_10l"

    #model, history = teach_img_based(dataset, _epochs=3000, _batch_size=512, _learning_rate=0.0001, num_hidden_layers=10, num_neurons=128, val_split=0.2, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")
    model, history = teach_img_based(dataset, _epochs=3000, _batch_size=64, _learning_rate=0.0001, num_hidden_layers=10, num_neurons=128, val_split=0.2, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model)
    plot_history(history, "RMSE during training", "Loss - during training", model_filename + "_rmse.png", model_filename + "_loss.png")

def train_nn_img_based_custom_test_set(train_dataset_filename, test_dataset_filename, network_filename):
    ''' train_nn_img_based '''

    print("Model training: Image based Custom Test Set")

    dataset_train = load_dataset(train_dataset_filename, "hdr_merge/")
    dataset_test  = load_dataset(test_dataset_filename,  "hdr_merge/")

    model_filename = network_filename

    model, history = teach_img_based_custom_test_set(dataset_train, dataset_test, _epochs=3000, _batch_size=64, _learning_rate=0.0001, num_hidden_layers=10, num_neurons=128, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model)
    plot_history(history, "RMSE during training", "Loss - during training", model_filename + "_rmse.png", model_filename + "_loss.png")

def transfer_learn_nn_img_based():
    ''' transfer_learn_nn_img_based '''

    print("Model training: Transfer learning")

    dataset = load_dataset("dataset_py_hdr.txt", "hdr_merge/")
    model_filename = "nn_img_based_tf_real_256_256"

    model = load_model("nn_img_based_synth_256_256_10l-84.h5", custom_objects={'rmse': rmse})
    model_tf, history = transfer_learn_model_img_based(model, dataset, _epochs=100, _batch_size=64, _learning_rate=0.0001, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model_tf)

    plot_history(history, "RMSE during training", "Loss - during training", model_filename + "_rmse.png", model_filename + "_loss.png")

def transfer_learn_nn_img_based_custom_test_set(train_dataset_filename, test_dataset_filename, network_filename):
    ''' transfer_learn_nn_img_based '''

    print("Model training: Transfer learning Custom Test Set")

    dataset_train = load_dataset(train_dataset_filename, "hdr_merge/")
    dataset_test  = load_dataset(test_dataset_filename, "hdr_merge/")

    model_filename = network_filename

    model = load_model("nn_img_based_synth_256_256_10l-84.h5", custom_objects={'rmse': rmse})
    model_tf, history = transfer_learn_model_img_based_custom_test_set(model, dataset_train, dataset_test, _epochs=3000, _batch_size=64, _learning_rate=0.0001, enable_checkpoints=True, checkpoint_filepath=model_filename+"-{epoch:02d}.h5")

    save_nn_model(model_filename + ".h5", model_tf)

    plot_history(history, "RMSE during training", "Loss - during training", model_filename + "_rmse.png", model_filename + "_loss.png")

def export_keras2cpp(filename, dir=""):
    from keras2cpp import export_model
    model = load_model(dir + filename + ".h5", custom_objects={'rmse': rmse})
    model.summary()
    export_model(model, filename + ".model")

#####################################################
##                   MAIN APP                      ##
#####################################################
os.environ['CUDA_VISIBLE_DEVICES'] = '-1' # 0 - Enable GPU support, -1 - Disable GPU support

# (Un)comment one of the below functions to train a model

# train_nn_single_planet()
# export_keras2cpp("nn_lut_512_128_64_64_64_rm_earth_3layers-390")

# train_nn_multi_planets()
# train_nn_multi_planets_with_tf()
# train_nn_img_based()
# train_nn_img_based_custom_test_set("dataset_py_hdr_even_train.txt", "dataset_py_hdr_even_test.txt", "nn_img_based_real_even")
# train_nn_img_based_custom_test_set("dataset_py_hdr_missing_11_15_train.txt", "dataset_py_hdr_missing_11_15_test.txt", "nn_img_based_real_missing_11_15")
# transfer_learn_nn_img_based()
# transfer_learn_nn_img_based_custom_test_set("dataset_py_hdr_even_train.txt",          "dataset_py_hdr_even_test.txt",          "nn_img_based_tf_real_256_256_even")
# transfer_learn_nn_img_based_custom_test_set("dataset_py_hdr_missing_11_15_train.txt", "dataset_py_hdr_missing_11_15_test.txt", "nn_img_based_tf_real_256_256_missing_11_15")

# Use export_keras2cpp to export keras model to a file that can be opened with C++ Atmosphere framework

# export_keras2cpp("nn_img_based_synth_256_256_10l-84", dir="saved_nn_models/image_based/5params/")
# export_keras2cpp("nn_img_based_real_256_256_10l-88")
# export_keras2cpp("nn_img_based_tf_real_256_256-81")
export_keras2cpp("saved_nn_models/multi_planets/nn_lut_512_128_128_128_128_rm_multi_planets-100")
