import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from sklearn.metrics import accuracy_score
from sklearn import metrics

data = pd.read_csv("/home/someone/work/data/combine_data/combined_timer.csv")
print(data.head())
print(data.shape)
headers = list(data.columns.values)

X = (data.to_numpy())[:, :15]
y = data.pop("mal_beni").values

for number in np.unique(y):
    plt.figure(1)
    data["VMA"].iloc[np.where(y == number)[0]].hist(bins=30)

np.where(y == number)[0]

plt.show(block=True)
    
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=.25)

clf = SVC(kernel="linear")
clf.fit(X_train, y_train)

y_pred = clf.predict(X_test)

print("-----accuracy-----")
print(accuracy_score(y_test, y_pred))

print("------y-test------")

print(y_test.mean())
print(metrics.confusion_matrix(y_test, y_pred))

print("-----precision------")
print(metrics.precision_score(y_test, y_pred))

print("-----recall-----")
print(metrics.recall_score(y_test, y_pred))

print("-----f1 score-----")
print(metrics.f1_score(y_test, y_pred))
